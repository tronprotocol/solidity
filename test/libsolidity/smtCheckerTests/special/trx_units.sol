contract D {
	function f() public pure {
		assert(1000000 sun == 1 trx);
		assert(100000 sun == 1 trx);
		assert(1 sun == 1);
		assert(2 sun == 1);
		assert(2 trx == 2000000 sun);
		assert(2 trx == 200000 sun);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (75-102): CHC: Assertion violation happens here.
// Warning 6328: (128-146): CHC: Assertion violation happens here.
// Warning 6328: (182-209): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
