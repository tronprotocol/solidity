contract Test {
    function f(uint[][] calldata) external { }
    function g(uint[][1] calldata) external { }
}
// ----
// TypeError 4957: (31-48): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
// TypeError 4957: (78-96): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
