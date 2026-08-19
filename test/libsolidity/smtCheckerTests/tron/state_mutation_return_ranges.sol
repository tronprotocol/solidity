contract C {
	function withdrawReward() external {
		assert(withdrawreward() <= type(uint256).max);
	}

	function cancelAllUnfreezeV2() external {
		assert(cancelallunfreezev2() <= type(uint256).max);
	}

	function withdrawExpireUnfreeze() external {
		assert(withdrawexpireunfreeze() <= type(uint256).max);
	}
}
// ====
// SMTEngine: chc
// SMTShowProvedSafe: yes
// ----
// Warning 4588: (60-76): Assertion checker does not yet implement this type of function call. Its state effects are modeled conservatively.
// Warning 4588: (156-177): Assertion checker does not yet implement this type of function call. Its state effects are modeled conservatively.
// Warning 4588: (260-284): Assertion checker does not yet implement this type of function call. Its state effects are modeled conservatively.
// Info 9576: (53-98): CHC: Assertion violation check is safe!
// Info 9576: (149-199): CHC: Assertion violation check is safe!
// Info 9576: (253-306): CHC: Assertion violation check is safe!
