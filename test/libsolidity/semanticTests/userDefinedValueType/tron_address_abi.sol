pragma abicoder v2;
type MyAddress is address;
type MyPayableAddress is address payable;
type MyUint24 is uint24;
type MyBool is bool;
type MyBytes2 is bytes2;

contract C {
    // Payable entry points let the generic EVM test host exercise the ABI decoder
    // without the TRON-specific non-payable CALLTOKENID/CALLTOKENVALUE checks.
    constructor() payable {}

    function plain(address a) external payable returns (address) { return a; }
    function wrapped(MyAddress a) external payable returns (MyAddress) { return a; }
    function raw(MyAddress a) external payable returns (uint256 value) {
        assembly { value := a }
    }
    function wrappedPayable(MyPayableAddress a) external payable returns (MyPayableAddress) { return a; }
    function decode(bytes memory data) external payable returns (MyAddress) {
        return abi.decode(data, (MyAddress));
    }
    function calldataArray(MyAddress[] calldata data) external payable returns (MyAddress) { return data[0]; }
    function memoryArray(MyAddress[] memory data) external payable returns (MyAddress) { return data[0]; }

    function integer(MyUint24 value) external payable returns (MyUint24) { return value; }
    function boolean(MyBool value) external payable returns (MyBool) { return value; }
    function fixedBytes(MyBytes2 value) external payable returns (MyBytes2) { return value; }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// plain(address): 0x1234 -> 0x1234
// plain(address): 0x410000000000000000000000000000000000001234 -> 0x1234
// plain(address): 0x01410000000000000000000000000000000000001234 -> FAILURE
// wrapped(address): 0x1234 -> 0x1234
// wrapped(address): 0x410000000000000000000000000000000000001234 -> 0x1234
// wrapped(address): 0xff0000000000000000000000000000000000001234 -> 0x1234
// wrapped(address): 0x01410000000000000000000000000000000000001234 -> FAILURE
// raw(address): 0x410000000000000000000000000000000000001234 -> 0x1234
// wrappedPayable(address): 0x410000000000000000000000000000000000001234 -> 0x1234
// decode(bytes): 0x20, 0x20, 0x1234 -> 0x1234
// decode(bytes): 0x20, 0x20, 0x410000000000000000000000000000000000001234 -> 0x1234
// decode(bytes): 0x20, 0x20, 0x01410000000000000000000000000000000000001234 -> FAILURE
// calldataArray(address[]): 0x20, 1, 0x1234 -> 0x1234
// calldataArray(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// calldataArray(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// memoryArray(address[]): 0x20, 1, 0x1234 -> 0x1234
// memoryArray(address[]): 0x20, 1, 0x410000000000000000000000000000000000001234 -> 0x1234
// memoryArray(address[]): 0x20, 1, 0x01410000000000000000000000000000000000001234 -> FAILURE
// integer(uint24): 0xffffff -> 0xffffff
// integer(uint24): 0x1000000 -> FAILURE
// boolean(bool): 1 -> true
// boolean(bool): 2 -> FAILURE
// fixedBytes(bytes2): 0x1234000000000000000000000000000000000000000000000000000000000000 -> 0x1234000000000000000000000000000000000000000000000000000000000000
// fixedBytes(bytes2): 0x1234000000000000000000000000000000000000000000000000000000000001 -> FAILURE
