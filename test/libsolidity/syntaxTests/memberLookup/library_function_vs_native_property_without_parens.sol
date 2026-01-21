// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.8.0;

// Test that native property is preferred over library function when accessed without parentheses.
// This allows Tron's native address.isContract property to work alongside library functions.
library Address {
    function isContract(address account) internal view returns (bool) {
        return account.code.length > 0;
    }
}

contract C {
    using Address for address;

    function check(address a) public view returns (bool) {
        // Without parentheses - should use native property
        return a.isContract;
    }
}
// ----
