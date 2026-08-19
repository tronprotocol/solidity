contract C {
	function chainParameters() external view {
		assert(chain.totalNetLimit <= type(uint64).max);
		assert(chain.totalNetWeight <= type(uint64).max);
		assert(chain.totalEnergyCurrentLimit <= type(uint64).max);
		assert(chain.totalEnergyWeight <= type(uint64).max);
		assert(chain.unfreezeDelayDays <= type(uint64).max);
	}

	function tokenCallParameterRanges() external payable {
		assert(msg.tokenvalue <= type(uint256).max);
		assert(msg.tokenid <= type(trcToken).max);
	}

	function nonPayableTokenCallParameters() external view {
		(uint256 trxValue, uint256 tokenValue, trcToken tokenId) = readTokenCallParameters();
		assert(trxValue == 0);
		assert(tokenValue == 0);
		assert(tokenId == 0);
	}

	function readTokenCallParameters() internal view returns (uint256, uint256, trcToken) {
		return (msg.value, msg.tokenvalue, msg.tokenid);
	}
}
// ====
// SMTEngine: chc
// SMTShowProvedSafe: yes
// ----
// Info 9576: (59-106): CHC: Assertion violation check is safe!
// Info 9576: (110-158): CHC: Assertion violation check is safe!
// Info 9576: (162-219): CHC: Assertion violation check is safe!
// Info 9576: (223-274): CHC: Assertion violation check is safe!
// Info 9576: (278-329): CHC: Assertion violation check is safe!
// Info 9576: (393-436): CHC: Assertion violation check is safe!
// Info 9576: (440-481): CHC: Assertion violation check is safe!
// Info 9576: (635-656): CHC: Assertion violation check is safe!
// Info 9576: (660-683): CHC: Assertion violation check is safe!
// Info 9576: (687-707): CHC: Assertion violation check is safe!
