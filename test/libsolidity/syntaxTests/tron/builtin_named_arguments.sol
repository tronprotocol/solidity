contract C {
    function f(bytes[] memory signatures) public view {
        validatemultisign({
            account: address(this),
            permissionId: 0,
            content: bytes32(0),
            signatures: signatures
        });
        isSrCandidate({srCandidate: address(this)});
        voteCount({voter: address(this), srCandidate: address(this)});
        totalVoteCount({voter: address(this)});
        receivedVoteCount({srCandidate: address(this)});
        usedVoteCount({voter: address(this)});
    }
}
// ----
