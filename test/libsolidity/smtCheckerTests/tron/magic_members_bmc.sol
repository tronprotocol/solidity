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
// SMTEngine: bmc
// SMTShowProvedSafe: yes
// ----
// Info 2961: (59-106): BMC: Assertion violation check is safe!
// Info 2961: (110-158): BMC: Assertion violation check is safe!
// Info 2961: (162-219): BMC: Assertion violation check is safe!
// Info 2961: (223-274): BMC: Assertion violation check is safe!
// Info 2961: (278-329): BMC: Assertion violation check is safe!
// Info 2961: (393-436): BMC: Assertion violation check is safe!
// Info 2961: (440-481): BMC: Assertion violation check is safe!
// Info 2961: (635-656): BMC: Assertion violation check is safe!
// Info 2961: (660-683): BMC: Assertion violation check is safe!
// Info 2961: (687-707): BMC: Assertion violation check is safe!
