    contract C {
        function f() public {
            // Invalid number
            78901234567890123456789012345678901234567890123456789345678901234567890012345678012345678901234567;
            [1, 78901234567890123456789012345678901234567890123456789345678901234567890012345678012345678901234567];
        }
    }
// ----
// TypeError: (89-187): Invalid rational number.
// TypeError: (205-303): Invalid rational number.