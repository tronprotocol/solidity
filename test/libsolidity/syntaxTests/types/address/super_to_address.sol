contract C {
    function f() public pure {
        address(super);
    }
}
// ----
// TypeError 9640: (52-66): Explicit type conversion not allowed from "contract super C" to "address".
