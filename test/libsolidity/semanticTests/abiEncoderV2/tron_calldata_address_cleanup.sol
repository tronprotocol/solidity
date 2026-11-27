pragma abicoder v2;
interface I {}
type MyAddress is address;
type MyPayableAddress is address payable;

struct S {
    address a;
    address payable p;
    I c;
    MyAddress m;
    MyPayableAddress mp;
}

contract C {
    address[] storedAddrs;
    address[2] storedFixed;
    address payable[] storedPayables;
    I[] storedContracts;
    S storedStruct;

    // Payable entry points let the generic EVM test host exercise calldata reads
    // and storage copies without TRON non-payable CALLTOKENID/CALLTOKENVALUE checks.
    constructor() payable {}

    function rawDynamic(address[] calldata a) external payable returns (uint256 value) {
        address x = a[0];
        assembly { value := x }
    }

    function storeDynamicRaw(address[] calldata a) external payable returns (uint256 value) {
        storedAddrs = a;
        uint256 slot;
        assembly {
            mstore(0, storedAddrs.slot)
            slot := keccak256(0, 32)
            value := sload(slot)
        }
    }

    function rawStatic(address[2] calldata a) external payable returns (uint256 v0, uint256 v1) {
        address x0 = a[0];
        address x1 = a[1];
        assembly {
            v0 := x0
            v1 := x1
        }
    }

    function storeStaticRaw(address[2] calldata a) external payable returns (uint256 v0, uint256 v1) {
        storedFixed = a;
        assembly {
            v0 := sload(storedFixed.slot)
            v1 := sload(add(storedFixed.slot, 1))
        }
    }

    function rawPayable(address payable[] calldata a) external payable returns (uint256 value) {
        address payable x = a[0];
        assembly { value := x }
    }

    function storePayableRaw(address payable[] calldata a) external payable returns (uint256 value) {
        storedPayables = a;
        uint256 slot;
        assembly {
            mstore(0, storedPayables.slot)
            slot := keccak256(0, 32)
            value := sload(slot)
        }
    }

    function rawContract(I[] calldata a) external payable returns (uint256 value) {
        I x = a[0];
        assembly { value := x }
    }

    function storeContractRaw(I[] calldata a) external payable returns (uint256 value) {
        storedContracts = a;
        uint256 slot;
        assembly {
            mstore(0, storedContracts.slot)
            slot := keccak256(0, 32)
            value := sload(slot)
        }
    }

    function rawStruct(S calldata s)
        external
        payable
        returns (uint256 va, uint256 vp, uint256 vc, uint256 vm, uint256 vmp)
    {
        address a = s.a;
        address payable p = s.p;
        I c = s.c;
        MyAddress m = s.m;
        MyPayableAddress mp = s.mp;
        assembly {
            va := a
            vp := p
            vc := c
            vm := m
            vmp := mp
        }
    }

    function storeStructRaw(S calldata s)
        external
        payable
        returns (uint256 va, uint256 vp, uint256 vc, uint256 vm, uint256 vmp)
    {
        storedStruct = s;
        assembly {
            va := sload(storedStruct.slot)
            vp := sload(add(storedStruct.slot, 1))
            vc := sload(add(storedStruct.slot, 2))
            vm := sload(add(storedStruct.slot, 3))
            vmp := sload(add(storedStruct.slot, 4))
        }
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// rawDynamic(address[]): 0x20, 1, 0x1234 -> 0x1234
// rawDynamic(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// rawDynamic(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// rawDynamic(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// storeDynamicRaw(address[]): 0x20, 1, 0x1234 -> 0x1234
// storeDynamicRaw(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// storeDynamicRaw(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// storeDynamicRaw(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// rawStatic(address[2]): 0x410000000000000000000000000000000000001234, 0xff0000000000000000000000000000000000005678 -> 0x1234, 0x5678
// rawStatic(address[2]): 0x01410000000000000000000000000000000000001234, 0x5678 -> FAILURE
// storeStaticRaw(address[2]): 0x410000000000000000000000000000000000001234, 0xff0000000000000000000000000000000000005678 -> 0x1234, 0x5678
// storeStaticRaw(address[2]): 0x01410000000000000000000000000000000000001234, 0x5678 -> FAILURE
// rawPayable(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// rawPayable(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// rawPayable(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// storePayableRaw(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// storePayableRaw(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// storePayableRaw(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// rawContract(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// rawContract(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// rawContract(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// storeContractRaw(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// storeContractRaw(address[]): 0x20, 1, 0xff0000000000000000000000000000000000001234 -> 0x1234
// storeContractRaw(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// rawStruct((address,address,address,address,address)): 0x410000000000000000000000000000000000001111, 0x410000000000000000000000000000000000002222, 0x410000000000000000000000000000000000003333, 0xff0000000000000000000000000000000000004444, 0x410000000000000000000000000000000000005555 -> 0x1111, 0x2222, 0x3333, 0x4444, 0x5555
// rawStruct((address,address,address,address,address)): 0x01410000000000000000000000000000000000001111, 0x2222, 0x3333, 0x4444, 0x5555 -> FAILURE
// storeStructRaw((address,address,address,address,address)): 0x410000000000000000000000000000000000001111, 0x410000000000000000000000000000000000002222, 0x410000000000000000000000000000000000003333, 0xff0000000000000000000000000000000000004444, 0x410000000000000000000000000000000000005555 -> 0x1111, 0x2222, 0x3333, 0x4444, 0x5555
// storeStructRaw((address,address,address,address,address)): 0x01410000000000000000000000000000000000001111, 0x2222, 0x3333, 0x4444, 0x5555 -> FAILURE
