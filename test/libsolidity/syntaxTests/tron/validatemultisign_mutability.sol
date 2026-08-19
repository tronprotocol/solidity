contract C {
    function f() public pure returns (bool) {
        return validatemultisign(address(0), 0, bytes32(0), new bytes[](0));
    }
}
// ----
// TypeError 2527: (74-134): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
