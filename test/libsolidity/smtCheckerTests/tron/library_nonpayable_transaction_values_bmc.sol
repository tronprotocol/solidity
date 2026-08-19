library L {
	function check() public view {
		assert(msg.value == 0);
		assert(msg.tokenvalue == 0);
		assert(msg.tokenid == 0);
	}
}
// ====
// SMTEngine: bmc
// SMTIgnoreCex: yes
// ----
// Warning 4661: (46-68): BMC: Assertion violation happens here.
// Warning 4661: (72-99): BMC: Assertion violation happens here.
// Warning 4661: (103-127): BMC: Assertion violation happens here.
