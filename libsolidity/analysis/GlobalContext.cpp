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
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#include <libsolidity/analysis/GlobalContext.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <memory>

using namespace std;

namespace solidity::frontend
{

/// Magic variables get negative ids for easy differentiation
int magicVariableToID(std::string const& _name)
{
	if (_name == "abi") return -1;
	else if (_name == "addmod") return -2;
	else if (_name == "assert") return -3;
	else if (_name == "block") return -4;
	else if (_name == "blockhash") return -5;
	else if (_name == "ecrecover") return -6;
	else if (_name == "gasleft") return -7;
	else if (_name == "keccak256") return -8;
	else if (_name == "log0") return -10;
	else if (_name == "log1") return -11;
	else if (_name == "log2") return -12;
	else if (_name == "log3") return -13;
	else if (_name == "log4") return -14;
	else if (_name == "msg") return -15;
	else if (_name == "mulmod") return -16;
	else if (_name == "now") return -17;
	else if (_name == "require") return -18;
	else if (_name == "revert") return -19;
	else if (_name == "ripemd160") return -20;
	else if (_name == "selfdestruct") return -21;
	else if (_name == "sha256") return -22;
	else if (_name == "sha3") return -23;
	else if (_name == "suicide") return -24;
	else if (_name == "super") return -25;
	else if (_name == "tx") return -26;
	else if (_name == "type") return -27;
	else if (_name == "this") return -28;
    else if (_name == "verifyMintProof") return -29;
    else if (_name == "verifyBurnProof") return -30;
    else if (_name == "verifyTransferProof") return -31;
    else if (_name == "pedersenHash") return -32;
    else if (_name == "batchvalidatesign") return -33;
    else if (_name == "validatemultisign") return -34;
    else if (_name == "freeze") return -35;
    else if (_name == "unfreeze") return -36;
    else if (_name == "freezeExpireTime") return -37;
	else
		solAssert(false, "Unknown magic variable: \"" + _name + "\".");
}

inline vector<shared_ptr<MagicVariableDeclaration const>> constructMagicVariables()
{
	static auto const magicVarDecl = [](string const& _name, Type const* _type) {
		return make_shared<MagicVariableDeclaration>(magicVariableToID(_name), _name, _type);
	};

	return {
		magicVarDecl("abi", TypeProvider::magic(MagicType::Kind::ABI)),
		magicVarDecl("addmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::AddMod, false, StateMutability::Pure)),
		magicVarDecl("assert", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Assert, false, StateMutability::Pure)),
		magicVarDecl("block", TypeProvider::magic(MagicType::Kind::Block)),
		magicVarDecl("blockhash", TypeProvider::function(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlockHash, false, StateMutability::View)),
		magicVarDecl("ecrecover", TypeProvider::function(strings{"bytes32", "uint8", "bytes32", "bytes32"}, strings{"address"}, FunctionType::Kind::ECRecover, false, StateMutability::Pure)),
		magicVarDecl("gasleft", TypeProvider::function(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft, false, StateMutability::View)),
		magicVarDecl("keccak256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVarDecl("log0", TypeProvider::function(strings{"bytes32"}, strings{}, FunctionType::Kind::Log0)),
		magicVarDecl("log1", TypeProvider::function(strings{"bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log1)),
		magicVarDecl("log2", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log2)),
		magicVarDecl("log3", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log3)),
		magicVarDecl("log4", TypeProvider::function(strings{"bytes32", "bytes32", "bytes32", "bytes32", "bytes32"}, strings{}, FunctionType::Kind::Log4)),
		magicVarDecl("msg", TypeProvider::magic(MagicType::Kind::Message)),
		magicVarDecl("mulmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::MulMod, false, StateMutability::Pure)),
		magicVarDecl("now", TypeProvider::uint256()),
		magicVarDecl("require", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVarDecl("require", TypeProvider::function(strings{"bool", "string memory"}, strings{}, FunctionType::Kind::Require, false, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings(), strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings{"string memory"}, strings(), FunctionType::Kind::Revert, false, StateMutability::Pure)),
		magicVarDecl("ripemd160", TypeProvider::function(strings{"bytes memory"}, strings{"bytes20"}, FunctionType::Kind::RIPEMD160, false, StateMutability::Pure)),
		magicVarDecl("selfdestruct", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVarDecl("sha256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::SHA256, false, StateMutability::Pure)),
		magicVarDecl("sha3", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, false, StateMutability::Pure)),
		magicVarDecl("suicide", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
                    magicVarDecl("freeze", TypeProvider::function(strings{"uint", "uint"}, strings{"bool"}, FunctionType::Kind::Freeze, false, StateMutability::NonPayable)),
                    magicVarDecl("unfreeze", TypeProvider::function(strings{"uint"}, strings{"bool"}, FunctionType::Kind::Unfreeze, false, StateMutability::NonPayable)),
                    magicVarDecl("freezeExpireTime", TypeProvider::function(strings{"uint"}, strings{"uint"}, FunctionType::Kind::FreezeExpireTime, false, StateMutability::NonPayable)),
		magicVarDecl("tx", TypeProvider::magic(MagicType::Kind::Transaction)),
		// Accepts a MagicType that can be any contract type or an Integer type and returns a
		// MagicType. The TypeChecker handles the correctness of the input and output types.
		magicVarDecl("type", TypeProvider::function(
			strings{},
			strings{},
			FunctionType::Kind::MetaType,
			true,
			StateMutability::Pure
		)),
	};
}

        GlobalContext::GlobalContext(): m_magicVariables{constructMagicVariables()}
        {
            addBatchValidateSignMethod();
            addValidateMultiSignMethod();
            addVerifyMintProofMethod();
            addVerifyBurnProofMethod();
            addVerifyTransferProofMethod();
            addPedersenHashMethod();
        }

        void GlobalContext::addVerifyMintProofMethod() {
            TypePointers parameterTypes;
            //output bytes32[9]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(9)));
            //bindingSignature bytes32[2]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(2)));
            //value uint64
            parameterTypes.push_back(TypeProvider::uint(64));
            //signHash bytes32
            parameterTypes.push_back(TypeProvider::fixedBytes(32));
            //frontier bytes32[33]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(33)));
            //leafCount uint256
            parameterTypes.push_back(TypeProvider::uint256());

            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32)));
            strings parameterNames;
            parameterNames.push_back("output");
            parameterNames.push_back("bindingSignature");
            parameterNames.push_back("value");
            parameterNames.push_back("signHash");
            parameterNames.push_back("frontier");
            parameterNames.push_back("leafCount");

            strings returnParameterNames;
            returnParameterNames.push_back("msg");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("verifyMintProof"), "verifyMintProof", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::verifyMintProof,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)
            ));
        }


        void GlobalContext::addVerifyBurnProofMethod() {
            TypePointers parameterTypes;
            //input bytes32[10]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(10)));
            //spend_auth_sig bytes32[2]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(2)));
            //value uint64
            parameterTypes.push_back(TypeProvider::uint(64));
            //bindingSignature  bytes32[2]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(2)));
            //signHash bytes32
            parameterTypes.push_back(TypeProvider::fixedBytes(32));

            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::boolean());
            strings parameterNames;
            parameterNames.push_back("input");
            parameterNames.push_back("spend_auth_sig");
            parameterNames.push_back("value");
            parameterNames.push_back("bindingSignature");
            parameterNames.push_back("signHash");


            strings returnParameterNames;
            returnParameterNames.push_back("msg");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("verifyBurnProof"), "verifyBurnProof", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::verifyBurnProof,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)));

        }


        void GlobalContext::addVerifyTransferProofMethod() {
            TypePointers parameterTypes;
            //bytes32[10][] input
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
                                                         TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(10))
            ));
            //spend_auth_sig bytes32[2][]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
                                                         TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(2))
            ));
            //output bytes32[9][]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
                                                         TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(9))
            ));
            //bindingSignature bytes32[2]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(2)));
            //signHash bytes32
            parameterTypes.push_back(TypeProvider::fixedBytes(32));
            //value uint256
            parameterTypes.push_back(TypeProvider::uint(64));
            //frontier bytes32[33]
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(33)));
            //leafCount uint256
            parameterTypes.push_back(TypeProvider::uint256());

            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32)));
            strings parameterNames;
            parameterNames.push_back("input");
            parameterNames.push_back("spend_auth_sig");
            parameterNames.push_back("output");
            parameterNames.push_back("bindingSignature");
            parameterNames.push_back("signHash");
            parameterNames.push_back("value");
            parameterNames.push_back("frontier");
            parameterNames.push_back("leafCount");

            strings returnParameterNames;
            returnParameterNames.push_back("msg");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("verifyTransferProof"), "verifyTransferProof", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::verifyTransferProof,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)
            ));
        }

        void GlobalContext::addPedersenHashMethod() {
            TypePointers parameterTypes;
            //i uint32
            parameterTypes.push_back(TypeProvider::uint(32));
            //left bytes32
            parameterTypes.push_back(TypeProvider::fixedBytes(32));
            //right bytes32
            parameterTypes.push_back(TypeProvider::fixedBytes(32));


            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::fixedBytes(32));
            strings parameterNames;
            parameterNames.push_back("i");
            parameterNames.push_back("left");
            parameterNames.push_back("right");


            strings returnParameterNames;
            returnParameterNames.push_back("msg");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("pedersenHash"), "pedersenHash", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::pedersenHash,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)));

        }


        void GlobalContext::addBatchValidateSignMethod() {
            // bool multivalidatesign(bytes32 hash, bytes[] memory signatures, address[] memory addresses)
            TypePointers parameterTypes;
            parameterTypes.push_back(TypeProvider::fixedBytes(32));
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::bytesMemory()));
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::address()));

            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::fixedBytes(32));
            strings parameterNames;
            parameterNames.push_back("hash");
            parameterNames.push_back("signatures");
            parameterNames.push_back("addresses");
            strings returnParameterNames;
            returnParameterNames.push_back("ok");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("batchvalidatesign"), "batchvalidatesign", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::BatchValidateSign,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)
            ));
        }


        void GlobalContext::addValidateMultiSignMethod() {
            // bool multivalidatesign(bytes32 hash, bytes[] memory signatures, address[] memory addresses)
            TypePointers parameterTypes;
            parameterTypes.push_back(TypeProvider::address());
            parameterTypes.push_back(TypeProvider::uint256());
            parameterTypes.push_back(TypeProvider::fixedBytes(32));
            parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::bytesMemory()));

            TypePointers returnParameterTypes;
            returnParameterTypes.push_back(TypeProvider::boolean());
            strings parameterNames;
            parameterNames.push_back("address");
            parameterNames.push_back("permissonid");
            parameterNames.push_back("content");
            parameterNames.push_back("signatures");
            strings returnParameterNames;
            returnParameterNames.push_back("ok");

            m_magicVariables.push_back(make_shared<MagicVariableDeclaration>(magicVariableToID("validatemultisign"), "validatemultisign", TypeProvider::function(
                    parameterTypes,
                    returnParameterTypes,
                    parameterNames,
                    returnParameterNames,
                    FunctionType::Kind::ValidateMultiSign,
                    false,
                    StateMutability::Pure,
                    nullptr,
                    false,
                    false,
                    false,
                    false)
            ));
        }

        void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
        {
            m_currentContract = &_contract;
        }

vector<Declaration const*> GlobalContext::declarations() const
{
	vector<Declaration const*> declarations;
	declarations.reserve(m_magicVariables.size());
	for (ASTPointer<MagicVariableDeclaration const> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	return declarations;
}

MagicVariableDeclaration const* GlobalContext::currentThis() const
{
	if (!m_thisPointer[m_currentContract])
		m_thisPointer[m_currentContract] = make_shared<MagicVariableDeclaration>(magicVariableToID("this"), "this", TypeProvider::contract(*m_currentContract));
	return m_thisPointer[m_currentContract].get();

}

MagicVariableDeclaration const* GlobalContext::currentSuper() const
{
	if (!m_superPointer[m_currentContract])
		m_superPointer[m_currentContract] = make_shared<MagicVariableDeclaration>(magicVariableToID("super"), "super", TypeProvider::contract(*m_currentContract, true));
	return m_superPointer[m_currentContract].get();
}

}
