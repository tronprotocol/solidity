contract C {
    function f(uint256 amount) public {
        uint256 balanceBefore = address(this).balance;
        payable(address(this)).freeze(amount, 0);
        assert(address(this).balance == balanceBefore);
    }
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4588: (116-156): Assertion checker does not yet implement this type of function call. Its state effects are modeled conservatively.
// Warning 6328: (166-212): CHC: Assertion violation happens here.
