/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <libevmasm/GasMeter.h>

#include <libevmasm/KnownState.h>

#include <algorithm>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;

GasMeter::GasConsumption& GasMeter::GasConsumption::operator+=(GasConsumption const& _other)
{
	if (_other.isInfinite && !isInfinite)
		*this = infinite();
	if (isInfinite)
		return *this;
	bigint v = bigint(value) + _other.value;
	if (v > std::numeric_limits<u256>::max())
		*this = infinite();
	else
		value = u256(v);
	return *this;
}

GasMeter::GasConsumption GasMeter::estimateMax(AssemblyItem const& _item, bool _includeExternalCosts)
{
	GasConsumption gas;
	switch (_item.type())
	{
	case Push:
		gas = pushGas(_item.data(), m_evmVersion);
		break;
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
	case PushDeployTimeAddress:
		gas = runGas(Instruction::PUSH1, m_evmVersion);
		break;
	case Tag:
		gas = runGas(Instruction::JUMPDEST, m_evmVersion);
		break;
	case Operation:
	{
		ExpressionClasses& classes = m_state->expressionClasses();
		switch (_item.instruction())
		{
		case Instruction::SSTORE:
		{
			ExpressionClasses::Id slot = m_state->relativeStackElement(0);
			ExpressionClasses::Id value = m_state->relativeStackElement(-1);
			if (classes.knownZero(value) || (
				m_state->storageContent().count(slot) &&
				classes.knownNonZero(m_state->storageContent().at(slot))
			))
				gas = GasCosts::sstoreResetGasInTVM; //@todo take refunds into account
			else
				gas = GasCosts::sstoreSetGasInTVM;
			break;
		}
		case Instruction::SLOAD:
			gas = GasCosts::sloadGasInTVM;
			break;
		case Instruction::RETURN:
		case Instruction::REVERT:
			gas = runGas(_item.instruction(), m_evmVersion);
			gas += memoryGas(0, -1);
			break;
		case Instruction::MLOAD:
		case Instruction::MSTORE:
			gas = runGas(_item.instruction(), m_evmVersion);
			gas += memoryGas(m_state->relativeStackElement(0), u256(32));
			break;
		case Instruction::MSTORE8:
			gas = runGas(_item.instruction(), m_evmVersion);
			gas += memoryGas(m_state->relativeStackElement(0), u256(1));
			break;
		case Instruction::KECCAK256:
			gas = GasCosts::keccak256Gas;
			gas += memoryGas(0, -1);
			gas += wordGas(GasCosts::keccak256WordGas, m_state->relativeStackElement(-1));
			break;
		case Instruction::CALLDATACOPY:
		case Instruction::CODECOPY:
		case Instruction::RETURNDATACOPY:
			gas = runGas(_item.instruction(), m_evmVersion);
			gas += memoryGas(0, -2);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-2));
			break;
		case Instruction::MCOPY:
		{
			gas = runGas(_item.instruction(), m_evmVersion);
			ExpressionClasses::Id sizeExpression = m_state->relativeStackElement(-2);
			if (!classes.knownZero(sizeExpression))
			{
				u256 const* source = classes.knownConstant(m_state->relativeStackElement(-1));
				u256 const* destination = classes.knownConstant(m_state->relativeStackElement(0));
				u256 const* size = classes.knownConstant(sizeExpression);
				if (!source || !destination || !size)
					gas = GasConsumption::infinite();
				else
					gas += memoryGas(bigint(std::max(*source, *destination)) + *size);
			}
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-2));
			break;
		}
		case Instruction::EXTCODESIZE:
			gas = GasCosts::extCodeSizeGasInTVM;
			break;
		case Instruction::EXTCODEHASH:
			gas = GasCosts::extCodeHashGasInTVM;
			break;
		case Instruction::EXTCODECOPY:
			gas = GasCosts::extCodeCopyGasInTVM;
			gas += memoryGas(-1, -3);
			gas += wordGas(GasCosts::copyGas, m_state->relativeStackElement(-3));
			break;
		case Instruction::LOG0:
		case Instruction::LOG1:
		case Instruction::LOG2:
		case Instruction::LOG3:
		case Instruction::LOG4:
		{
			gas = GasCosts::logGas + GasCosts::logTopicGas * getLogNumber(_item.instruction());
			gas += memoryGas(0, -1);
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
				gas += GasCosts::logDataGas * (*value);
			else
				gas = GasConsumption::infinite();
			break;
		}
		case Instruction::CALLTOKEN:
		case Instruction::CALL:
		case Instruction::CALLCODE:
		case Instruction::DELEGATECALL:
		case Instruction::STATICCALL:
		{
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::callGasInTVM;
				if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(0)))
					gas += (*value);
				else
					gas = GasConsumption::infinite();
				int valueSize = 1;
				if (_item.instruction() == Instruction::DELEGATECALL || _item.instruction() == Instruction::STATICCALL)
					valueSize = 0;
				else if (!classes.knownZero(m_state->relativeStackElement(-1 - valueSize)))
				{
					gas += GasCosts::callValueTransferGas;
					if (_item.instruction() == Instruction::CALL || _item.instruction() == Instruction::CALLTOKEN)
						gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
				}
				int tokenIdSize = 0;
				if (_item.instruction() == Instruction::CALLTOKEN)
					tokenIdSize = 1;
				gas += memoryGas(-2 - valueSize - tokenIdSize, -3 - valueSize - tokenIdSize);
				gas += memoryGas(-4 - valueSize - tokenIdSize, -5 - valueSize - tokenIdSize);
			}
			break;
		}
		case Instruction::SELFDESTRUCT:
			gas = GasCosts::selfdestructGasInTVM;
			gas += GasCosts::callNewAccountGas; // We very rarely know whether the address exists.
			break;
		case Instruction::CREATE:
		case Instruction::CREATE2:
			if (_includeExternalCosts)
				// We assume that we do not know the target contract and thus, the consumption is infinite.
				gas = GasConsumption::infinite();
			else
			{
				gas = GasCosts::createGas;
				gas += memoryGas(-1, -2);
				if (_item.instruction() == Instruction::CREATE2)
					gas += wordGas(GasCosts::create2WordGasInTVM, m_state->relativeStackElement(-2));
			}
			break;
		case Instruction::EXP:
			gas = GasCosts::expGas;
			if (u256 const* value = classes.knownConstant(m_state->relativeStackElement(-1)))
			{
				if (*value)
				{
					// Note: msb() counts from 0 and throws on 0 as input.
					unsigned const significantByteCount  = (static_cast<unsigned>(boost::multiprecision::msb(*value)) + 1u + 7u) / 8u;
					gas += GasCosts::expByteGasInTVM * significantByteCount;
				}
			}
			else
				gas += GasCosts::expByteGasInTVM * 32;
			break;
		case Instruction::BALANCE:
		case Instruction::TOKENBALANCE:
		case Instruction::ISCONTRACT:
			gas = GasCosts::balanceGasInTVM;
			break;
		case Instruction::NATIVEFREEZE:
			gas = GasCosts::freezeV1GasInTVM;
			gas += GasCosts::callNewAccountGas;
			break;
		case Instruction::NATIVEUNFREEZE:
			gas = GasCosts::freezeV1GasInTVM;
			break;
		case Instruction::NATIVEFREEZEEXPIRETIME:
			gas = GasCosts::freezeExpireTimeGasInTVM;
			break;
		case Instruction::NATIVEVOTE:
			gas = GasCosts::voteGasInTVM;
			// NATIVEVOTE reads two Solidity memory arrays. The stack length values are element
			// counts, not byte lengths, so include the array length slot and 32 bytes per element.
			gas += memoryGasForWordArray(-3, -2);
			gas += memoryGasForWordArray(-1, 0);
			break;
		case Instruction::NATIVEWITHDRAWREWARD:
			gas = GasCosts::withdrawRewardGasInTVM;
			break;
		case Instruction::NATIVEFREEZEBALANCEV2:
		case Instruction::NATIVEUNFREEZEBALANCEV2:
		case Instruction::NATIVECANCELALLUNFREEZEV2:
		case Instruction::NATIVEWITHDRAWEXPIREUNFREEZE:
		case Instruction::NATIVEDELEGATERESOURCE:
		case Instruction::NATIVEUNDELEGATERESOURCE:
			gas = GasCosts::freezeV2GasInTVM;
			break;
		case Instruction::CHAINID:
			gas = runGas(Instruction::CHAINID, m_evmVersion);
			break;
		case Instruction::SELFBALANCE:
			gas = runGas(Instruction::SELFBALANCE, m_evmVersion);
			break;
		default:
			gas = runGas(_item.instruction(), m_evmVersion);
			break;
		}
		break;
	}
	default:
		gas = GasConsumption::infinite();
		break;
	}

	m_state->feedItem(_item);
	return gas;
}

GasMeter::GasConsumption GasMeter::wordGas(u256 const& _multiplier, ExpressionClasses::Id _value)
{
	u256 const* value = m_state->expressionClasses().knownConstant(_value);
	if (!value)
		return GasConsumption::infinite();
	bigint gas = bigint(_multiplier) * ((bigint(*value) + 31) / 32);
	if (gas > std::numeric_limits<u256>::max())
		return GasConsumption::infinite();
	return GasConsumption(u256(gas));
}

GasMeter::GasConsumption GasMeter::memoryGas(bigint const& _position)
{
	if (
		_position < 0 ||
		_position > GasCosts::memorySizeLimitInTVM ||
		bigint(m_largestMemoryAccess) > GasCosts::memorySizeLimitInTVM
	)
		return GasConsumption::infinite();
	u256 const value = u256(_position);
	if (value < m_largestMemoryAccess)
		return GasConsumption(0);
	u256 previous = m_largestMemoryAccess;
	m_largestMemoryAccess = value;
	auto memGas = [=](u256 const& pos) -> u256
	{
		u256 size = (pos + 31) / 32;
		return GasCosts::memoryGas * size + size * size / GasCosts::quadCoeffDiv;
	};
	return memGas(value) - memGas(previous);
}

GasMeter::GasConsumption GasMeter::memoryGas(ExpressionClasses::Id _offset, u256 const& _size)
{
	u256 const* offset = m_state->expressionClasses().knownConstant(_offset);
	if (!offset)
		return GasConsumption::infinite();
	return memoryGas(bigint(*offset) + _size);
}

GasMeter::GasConsumption GasMeter::memoryGas(int _stackPosOffset, int _stackPosSize)
{
	ExpressionClasses& classes = m_state->expressionClasses();
	ExpressionClasses::Id offsetExpression = m_state->relativeStackElement(_stackPosOffset);
	ExpressionClasses::Id sizeExpression = m_state->relativeStackElement(_stackPosSize);
	if (classes.knownZero(sizeExpression))
		return GasConsumption(0);
	u256 const* offset = classes.knownConstant(offsetExpression);
	u256 const* size = classes.knownConstant(sizeExpression);
	if (!offset || !size)
		return GasConsumption::infinite();
	return memoryGas(bigint(*offset) + *size);
}

GasMeter::GasConsumption GasMeter::memoryGasForWordArray(int _stackPosOffset, int _stackPosElementCount)
{
	ExpressionClasses& classes = m_state->expressionClasses();
	// The TVM reads and charges the 32-byte length slot even for empty arrays,
	// so unlike memoryGas(int, int) there is no zero-size shortcut here.
	u256 const* offset = classes.knownConstant(m_state->relativeStackElement(_stackPosOffset));
	u256 const* elementCount = classes.knownConstant(m_state->relativeStackElement(_stackPosElementCount));
	if (!offset || !elementCount)
		return GasConsumption::infinite();
	bigint const byteSizeWithLengthSlot = bigint(*elementCount) * 32 + 32;
	return memoryGas(bigint(*offset) + byteSizeWithLengthSlot);
}

namespace
{
std::optional<unsigned> gasCostForTier(Tier _tier)
{
	switch (_tier)
	{
	case Tier::Zero:        return GasCosts::tier0Gas;
	case Tier::Base:        return GasCosts::tier1Gas;
	case Tier::RJump:       return GasCosts::tier1Gas;
	case Tier::RJumpI:      return GasCosts::rjumpiGas;
	case Tier::VeryLow:     return GasCosts::tier2Gas;
	case Tier::RetF:        return GasCosts::tier2Gas;
	case Tier::Low:         return GasCosts::tier3Gas;
	case Tier::CallF:       return GasCosts::tier3Gas;
	case Tier::JumpF:       return GasCosts::tier3Gas;
	case Tier::Mid:         return GasCosts::tier4Gas;
	case Tier::High:        return GasCosts::tier5Gas;
	case Tier::BlockHash:   return GasCosts::tier6Gas;
	case Tier::WarmAccess:  return GasCosts::warmStorageReadCost;

	case Tier::Special:
	case Tier::Invalid:
		return std::nullopt;
	}
	util::unreachable();
}
}

unsigned GasMeter::runGas(Instruction _instruction, langutil::EVMVersion _evmVersion)
{
	if (_instruction == Instruction::JUMPDEST)
		return 1;

	if (auto gasCost = gasCostForTier(instructionInfo(_instruction, _evmVersion).gasPriceTier))
		return *gasCost;
	solAssert(false, "Invalid gas tier for instruction " + instructionInfo(_instruction, _evmVersion).name);
}

unsigned GasMeter::pushGas(u256 _value, langutil::EVMVersion _evmVersion)
{
	return runGas(
		(_evmVersion.hasPush0() && _value == u256(0)) ? Instruction::PUSH0 : Instruction::PUSH1,
		_evmVersion
	);
}

unsigned GasMeter::swapGas(size_t _depth, langutil::EVMVersion _evmVersion)
{
	if (_depth <= 16)
		return runGas(evmasm::swapInstruction(static_cast<unsigned>(_depth)), _evmVersion);
	auto gasCost = gasCostForTier(instructionInfo(evmasm::Instruction::SWAPN, _evmVersion).gasPriceTier);
	solAssert(gasCost.has_value(), "Expected gas cost for SWAPN to be defined.");
	return *gasCost;
}

unsigned GasMeter::dupGas(size_t _depth, langutil::EVMVersion _evmVersion)
{
	if (_depth <= 16)
		return runGas(evmasm::swapInstruction(static_cast<unsigned>(_depth)), _evmVersion);
	auto gasCost = gasCostForTier(instructionInfo(evmasm::Instruction::DUPN, _evmVersion).gasPriceTier);
	solAssert(gasCost.has_value(), "Expected gas cost for DUPN to be defined.");
	return *gasCost;
}

u256 GasMeter::dataGas(bytes const& _data, bool _inCreation, langutil::EVMVersion _evmVersion)
{
	bigint gas = 0;
	if (_inCreation)
	{
		for (auto b: _data)
			gas += (b != 0) ? GasCosts::txDataNonZeroGas(_evmVersion) : GasCosts::txDataZeroGas;
	}
	else
		gas = bigint(GasCosts::createDataGas) * _data.size();
	solAssert(gas < bigint(u256(-1)), "Gas cost exceeds 256 bits.");
	return u256(gas);
}


u256 GasMeter::dataGas(uint64_t _length, bool _inCreation, langutil::EVMVersion _evmVersion)
{
	bigint gas = bigint(_length) * (_inCreation ? GasCosts::txDataNonZeroGas(_evmVersion) : GasCosts::createDataGas);
	solAssert(gas < bigint(u256(-1)), "Gas cost exceeds 256 bits.");
	return u256(gas);
}
