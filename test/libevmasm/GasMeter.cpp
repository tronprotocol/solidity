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
/**
 * Tests for gas estimation of TRON-specific instructions in libevmasm's GasMeter.
 */

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/GasMeter.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/KnownState.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

using namespace solidity::evmasm;
using namespace solidity::langutil;

namespace solidity::frontend::test
{

namespace
{
GasMeter::GasConsumption estimateInstruction(
	Instruction _instruction,
	AssemblyItems _arguments,
	bool _includeExternalCosts = true,
	EVMVersion _evmVersion = EVMVersion{}
)
{
	_arguments.emplace_back(_instruction);
	GasMeter meter(std::make_shared<KnownState>(), _evmVersion);
	GasMeter::GasConsumption gas;
	for (AssemblyItem const& item: _arguments)
		gas = meter.estimateMax(item, _includeExternalCosts);
	return gas;
}

AssemblyItems zeroArguments(size_t _count)
{
	return AssemblyItems(_count, AssemblyItem{u256(0)});
}

u256 memoryExpansionCost(u256 const& _byteSize)
{
	u256 const wordCount = (_byteSize + 31) / 32;
	return GasCosts::memoryGas * wordCount + wordCount * wordCount / GasCosts::quadCoeffDiv;
}

/// Feeds the four NATIVEVOTE arguments as constants (or CALLVALUE for an unknown
/// element count) and returns the estimate for the NATIVEVOTE item itself.
GasMeter::GasConsumption estimateVote(
	u256 const& _srOffset,
	u256 const& _srCount,
	u256 const& _tpOffset,
	std::optional<u256> const& _tpCount
)
{
	// KnownState keeps pointers into the fed items (feedItem's _copyItem defaults
	// to false), so they have to outlive the meter, as they do in real assembly.
	// NATIVEVOTE stack layout, top first: tpCount, tpOffset, srCount, srOffset.
	AssemblyItems items{
		_srOffset,
		_srCount,
		_tpOffset,
		_tpCount ? AssemblyItem(*_tpCount) : AssemblyItem(Instruction::CALLVALUE),
		Instruction::NATIVEVOTE
	};
	GasMeter meter(std::make_shared<KnownState>(), solidity::test::CommonOptions::get().evmVersion());
	GasMeter::GasConsumption gas;
	for (AssemblyItem const& item: items)
		gas = meter.estimateMax(item);
	return gas;
}
}

BOOST_AUTO_TEST_SUITE(EvmasmGasMeter)

BOOST_AUTO_TEST_CASE(tvm_fixed_instruction_prices_do_not_depend_on_evm_version)
{
	std::vector<std::tuple<Instruction, size_t, unsigned>> const fixedCosts{
		{Instruction::SLOAD, 1, GasCosts::sloadGasInTVM},
		{Instruction::BALANCE, 1, GasCosts::balanceGasInTVM},
		{Instruction::TOKENBALANCE, 2, GasCosts::balanceGasInTVM},
		{Instruction::ISCONTRACT, 1, GasCosts::balanceGasInTVM},
		{Instruction::EXTCODESIZE, 1, GasCosts::extCodeSizeGasInTVM},
		{Instruction::EXTCODECOPY, 4, GasCosts::extCodeCopyGasInTVM},
		{Instruction::EXTCODEHASH, 1, GasCosts::extCodeHashGasInTVM},
		{Instruction::NATIVEFREEZE, 3, GasCosts::freezeV1GasInTVM + GasCosts::callNewAccountGas},
		{Instruction::NATIVEUNFREEZE, 2, GasCosts::freezeV1GasInTVM},
		{Instruction::NATIVEFREEZEEXPIRETIME, 2, GasCosts::freezeExpireTimeGasInTVM},
		{Instruction::NATIVEWITHDRAWREWARD, 0, GasCosts::withdrawRewardGasInTVM},
		{Instruction::NATIVEFREEZEBALANCEV2, 2, GasCosts::freezeV2GasInTVM},
		{Instruction::NATIVEUNFREEZEBALANCEV2, 2, GasCosts::freezeV2GasInTVM},
		{Instruction::NATIVECANCELALLUNFREEZEV2, 0, GasCosts::freezeV2GasInTVM},
		{Instruction::NATIVEWITHDRAWEXPIREUNFREEZE, 0, GasCosts::freezeV2GasInTVM},
		{Instruction::NATIVEDELEGATERESOURCE, 3, GasCosts::freezeV2GasInTVM},
		{Instruction::NATIVEUNDELEGATERESOURCE, 3, GasCosts::freezeV2GasInTVM}
	};

	for (EVMVersion const& evmVersion: EVMVersion::allVersions())
		for (auto const& [instruction, argumentCount, expected]: fixedCosts)
		{
			GasMeter::GasConsumption gas = estimateInstruction(
				instruction,
				zeroArguments(argumentCount),
				true,
				evmVersion
			);
			BOOST_REQUIRE(!gas.isInfinite);
			BOOST_CHECK_EQUAL(gas.value, u256(expected));
		}
}

BOOST_AUTO_TEST_CASE(tvm_sstore_uses_java_tron_set_and_reset_prices)
{
	GasMeter::GasConsumption set = estimateInstruction(Instruction::SSTORE, {u256(1), u256(0)});
	BOOST_REQUIRE(!set.isInfinite);
	BOOST_CHECK_EQUAL(set.value, u256(GasCosts::sstoreSetGasInTVM));

	GasMeter::GasConsumption reset = estimateInstruction(Instruction::SSTORE, {u256(0), u256(0)});
	BOOST_REQUIRE(!reset.isInfinite);
	BOOST_CHECK_EQUAL(reset.value, u256(GasCosts::sstoreResetGasInTVM));
}

BOOST_AUTO_TEST_CASE(tvm_exp_uses_fixed_byte_price_for_all_evm_versions)
{
	for (EVMVersion const& evmVersion: EVMVersion::allVersions())
	{
		GasMeter::GasConsumption zeroExponent = estimateInstruction(
			Instruction::EXP,
			{u256(0), u256(2)},
			true,
			evmVersion
		);
		BOOST_REQUIRE(!zeroExponent.isInfinite);
		BOOST_CHECK_EQUAL(zeroExponent.value, u256(GasCosts::expGas));

		GasMeter::GasConsumption oneByteExponent = estimateInstruction(
			Instruction::EXP,
			{u256(1), u256(2)},
			true,
			evmVersion
		);
		BOOST_REQUIRE(!oneByteExponent.isInfinite);
		BOOST_CHECK_EQUAL(
			oneByteExponent.value,
			u256(GasCosts::expGas + GasCosts::expByteGasInTVM)
		);

		GasMeter::GasConsumption fullWidthExponent = estimateInstruction(
			Instruction::EXP,
			{u256(-1), u256(2)},
			true,
			evmVersion
		);
		BOOST_REQUIRE(!fullWidthExponent.isInfinite);
		BOOST_CHECK_EQUAL(
			fullWidthExponent.value,
			u256(GasCosts::expGas + 32 * GasCosts::expByteGasInTVM)
		);
	}

	GasMeter::GasConsumption unknownExponent = estimateInstruction(
		Instruction::EXP,
		{AssemblyItem(Instruction::CALLVALUE), AssemblyItem(u256(2))}
	);
	BOOST_REQUIRE(!unknownExponent.isInfinite);
	BOOST_CHECK_EQUAL(
		unknownExponent.value,
		u256(GasCosts::expGas + 32 * GasCosts::expByteGasInTVM)
	);
}

BOOST_AUTO_TEST_CASE(tvm_create2_charges_hash_cost_per_init_code_word)
{
	for (unsigned size: {0u, 1u, 32u, 33u})
	{
		GasMeter::GasConsumption create = estimateInstruction(
			Instruction::CREATE,
			{u256(size), u256(0), u256(0)},
			false
		);
		GasMeter::GasConsumption create2 = estimateInstruction(
			Instruction::CREATE2,
			{u256(0), u256(size), u256(0), u256(0)},
			false
		);
		BOOST_REQUIRE(!create.isInfinite);
		BOOST_REQUIRE(!create2.isInfinite);
		u256 const wordCount = (u256(size) + 31) / 32;
		u256 const expectedCreateCost = GasCosts::createGas + memoryExpansionCost(size);
		BOOST_CHECK_EQUAL(create.value, expectedCreateCost);
		BOOST_CHECK_EQUAL(
			create2.value,
			expectedCreateCost + GasCosts::create2WordGasInTVM * wordCount
		);
	}
}

BOOST_AUTO_TEST_CASE(tvm_memory_limit_and_address_overflow_are_unbounded)
{
	u256 const memoryLimit = GasCosts::memorySizeLimitInTVM;
	GasMeter::GasConsumption atLimit = estimateInstruction(
		Instruction::MSTORE8,
		{u256(1), memoryLimit - 1}
	);
	BOOST_REQUIRE(!atLimit.isInfinite);
	BOOST_CHECK_EQUAL(
		atLimit.value,
		u256(GasMeter::runGas(Instruction::MSTORE8, EVMVersion{})) + memoryExpansionCost(memoryLimit)
	);

	GasMeter::GasConsumption aboveLimit = estimateInstruction(
		Instruction::MSTORE8,
		{u256(1), memoryLimit}
	);
	BOOST_CHECK(aboveLimit.isInfinite);

	GasMeter::GasConsumption wrappedEnd = estimateInstruction(
		Instruction::MSTORE8,
		{u256(1), u256(-1)}
	);
	BOOST_CHECK(wrappedEnd.isInfinite);

	// As in java-tron's memNeeded(), a zero-sized access does not expand memory,
	// regardless of the offset value.
	GasMeter::GasConsumption zeroSizeAtMaxOffset = estimateInstruction(
		Instruction::RETURN,
		{u256(0), u256(-1)}
	);
	BOOST_REQUIRE(!zeroSizeAtMaxOffset.isInfinite);
	BOOST_CHECK_EQUAL(zeroSizeAtMaxOffset.value, u256(0));
}

BOOST_AUTO_TEST_CASE(mcopy_charges_memory_expansion_to_the_larger_end)
{
	GasMeter::GasConsumption overlappingRanges = estimateInstruction(
		Instruction::MCOPY,
		{u256(64), u256(0), u256(32)},
		true,
		EVMVersion::cancun()
	);
	BOOST_REQUIRE(!overlappingRanges.isInfinite);
	BOOST_CHECK_EQUAL(
		overlappingRanges.value,
		u256(GasMeter::runGas(Instruction::MCOPY, EVMVersion::cancun())) +
			2 * GasCosts::copyGas +
			memoryExpansionCost(96)
	);

	GasMeter::GasConsumption zeroSizeAtMaxOffsets = estimateInstruction(
		Instruction::MCOPY,
		{u256(0), u256(-1), u256(-1)},
		true,
		EVMVersion::cancun()
	);
	BOOST_REQUIRE(!zeroSizeAtMaxOffsets.isInfinite);
	BOOST_CHECK_EQUAL(
		zeroSizeAtMaxOffsets.value,
		u256(GasMeter::runGas(Instruction::MCOPY, EVMVersion::cancun()))
	);
}

BOOST_AUTO_TEST_CASE(tvm_call_family_uses_fixed_base_and_conditional_transfer_prices)
{
	for (Instruction instruction: {Instruction::DELEGATECALL, Instruction::STATICCALL})
	{
		GasMeter::GasConsumption gas = estimateInstruction(instruction, zeroArguments(6), false);
		BOOST_REQUIRE(!gas.isInfinite);
		BOOST_CHECK_EQUAL(gas.value, u256(GasCosts::callGasInTVM));
	}

	GasMeter::GasConsumption zeroValueCall = estimateInstruction(
		Instruction::CALL,
		{u256(0), u256(0), u256(0), u256(0), u256(0), u256(2), u256(0)},
		false
	);
	BOOST_REQUIRE(!zeroValueCall.isInfinite);
	BOOST_CHECK_EQUAL(zeroValueCall.value, u256(GasCosts::callGasInTVM));

	GasMeter::GasConsumption valueCall = estimateInstruction(
		Instruction::CALL,
		{u256(0), u256(0), u256(0), u256(0), u256(1), u256(2), u256(0)},
		false
	);
	BOOST_REQUIRE(!valueCall.isInfinite);
	BOOST_CHECK_EQUAL(
		valueCall.value,
		u256(GasCosts::callGasInTVM + GasCosts::callValueTransferGas + GasCosts::callNewAccountGas)
	);

	GasMeter::GasConsumption valueCallCode = estimateInstruction(
		Instruction::CALLCODE,
		{u256(0), u256(0), u256(0), u256(0), u256(1), u256(2), u256(0)},
		false
	);
	BOOST_REQUIRE(!valueCallCode.isInfinite);
	BOOST_CHECK_EQUAL(valueCallCode.value, u256(GasCosts::callGasInTVM + GasCosts::callValueTransferGas));
}

BOOST_AUTO_TEST_CASE(tvm_calltoken_uses_fixed_base_and_conditional_transfer_prices)
{
	GasMeter::GasConsumption zeroValue = estimateInstruction(
		Instruction::CALLTOKEN,
		{u256(0), u256(0), u256(0), u256(0), u256(1), u256(0), u256(2), u256(0)},
		false
	);
	BOOST_REQUIRE(!zeroValue.isInfinite);
	BOOST_CHECK_EQUAL(zeroValue.value, u256(GasCosts::callGasInTVM));

	GasMeter::GasConsumption nonZeroValue = estimateInstruction(
		Instruction::CALLTOKEN,
		{u256(0), u256(0), u256(0), u256(0), u256(1), u256(1), u256(2), u256(0)},
		false
	);
	BOOST_REQUIRE(!nonZeroValue.isInfinite);
	BOOST_CHECK_EQUAL(
		nonZeroValue.value,
		u256(GasCosts::callGasInTVM + GasCosts::callValueTransferGas + GasCosts::callNewAccountGas)
	);
}

BOOST_AUTO_TEST_CASE(tvm_selfdestruct_uses_fixed_price_plus_new_account_cost)
{
	// java-tron (EnergyCost.getSuicideCost3) charges SUICIDE_V2 plus
	// NEW_ACCT_CALL when the inheritor is a dead account. The estimator
	// conservatively always adds the new-account cost.
	GasMeter::GasConsumption gas = estimateInstruction(Instruction::SELFDESTRUCT, zeroArguments(1));
	BOOST_REQUIRE(!gas.isInfinite);
	BOOST_CHECK_EQUAL(gas.value, u256(GasCosts::selfdestructGasInTVM + GasCosts::callNewAccountGas));
}

BOOST_AUTO_TEST_CASE(nativevote_charges_memory_expansion_for_word_arrays)
{
	// java-tron (EnergyCost.getVoteWitnessCost2/3) charges per array
	// memNeeded(offset, 32 * count + 32) and expands once to the larger end.
	// Arrays at 128 and 256 with two elements each end at 224 and 352 bytes,
	// i.e. 7 and 11 words: 3 * 11 = 33 on top of the flat vote cost.
	GasMeter::GasConsumption gas = estimateVote(u256(128), u256(2), u256(256), u256(2));
	BOOST_REQUIRE(!gas.isInfinite);
	BOOST_CHECK_EQUAL(gas.value, u256(GasCosts::voteGasInTVM + 33));
}

BOOST_AUTO_TEST_CASE(nativevote_charges_length_slot_for_empty_arrays)
{
	// The TVM reads and charges the 32-byte length slot even when count == 0:
	// ends are 128+32 and 512+32 bytes, i.e. 5 and 17 words: 3 * 17 = 51.
	GasMeter::GasConsumption gas = estimateVote(u256(128), u256(0), u256(512), u256(0));
	BOOST_REQUIRE(!gas.isInfinite);
	BOOST_CHECK_EQUAL(gas.value, u256(GasCosts::voteGasInTVM + 51));
}

BOOST_AUTO_TEST_CASE(nativevote_with_unknown_element_count_is_unbounded)
{
	GasMeter::GasConsumption gas = estimateVote(u256(128), u256(2), u256(256), std::nullopt);
	BOOST_CHECK(gas.isInfinite);
}

BOOST_AUTO_TEST_CASE(nativevote_checks_memory_limit_without_u256_wraparound)
{
	u256 const maxElementCount = GasCosts::memorySizeLimitInTVM / 32 - 1;
	GasMeter::GasConsumption atLimit = estimateVote(u256(0), maxElementCount, u256(0), u256(0));
	BOOST_REQUIRE(!atLimit.isInfinite);
	BOOST_CHECK_EQUAL(
		atLimit.value,
		u256(GasCosts::voteGasInTVM) + memoryExpansionCost(GasCosts::memorySizeLimitInTVM)
	);

	GasMeter::GasConsumption aboveLimit = estimateVote(u256(0), maxElementCount + 1, u256(0), u256(0));
	BOOST_CHECK(aboveLimit.isInfinite);

	GasMeter::GasConsumption wrappedProduct = estimateVote(
		u256(0),
		u256(1) << 251,
		u256(0),
		u256(0)
	);
	BOOST_CHECK(wrappedProduct.isInfinite);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
