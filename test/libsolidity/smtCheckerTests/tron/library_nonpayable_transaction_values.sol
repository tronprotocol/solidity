library L {
	function check() public view {
		assert(msg.value == 0);
		assert(msg.tokenvalue == 0);
		assert(msg.tokenid == 0);
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// ----
// Warning 6328: (46-68): CHC: Assertion violation happens here.
// Warning 6328: (72-99): CHC: Assertion violation happens here.
// Warning 6328: (103-127): CHC: Assertion violation happens here.
