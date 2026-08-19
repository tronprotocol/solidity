contract C {
    function f() public view returns (bool, uint64) {
        bool valid = validatemultisign(address(0), 0, bytes32(0), new bytes[](0));
        return (valid, chain.totalNetLimit);
    }
}
// ====
// EVMVersion: <byzantium
// ----
// TypeError 9137: (88-148): This TRON builtin requires a Byzantium-compatible VM.
// TypeError 9137: (173-192): This TRON builtin requires a Byzantium-compatible VM.
