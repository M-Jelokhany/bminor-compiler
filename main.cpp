
#include <iostream>
#include "bminor/SourceCursor.h"

int main() {
    bminor::SourceCursor cursor("ab", "test.bminor");

    // Test 1: Initial character
    if (cursor.peek() != 'a') {
        std::cerr << "Test 1 failed!\n";
        return 1;
    }
    std::cout << "Test 1 passed!\n";

    // Test 2: Move to next character
    if (cursor.advance() != 'a' || cursor.peek() != 'b') {
        std::cerr << "Test 2 failed!\n";
        return 1;
    }
    std::cout << "Test 2 passed!\n";

    // Test 3: Check source location
    auto loc = cursor.location();

    if (loc.line != 1 || loc.column != 2 || loc.offset != 1) {
        std::cerr << "Test 3 failed!\n";
        return 1;
    }
    std::cout << "Test 3 passed!\n";

    // Test 4: Reach end of source
    cursor.advance();

    if (!cursor.atEnd()) {
        std::cerr << "Test 4 failed!\n";
        return 1;
    }
    std::cout << "Test 4 passed!\n";


    // Test 5: Newline
    bminor::SourceCursor newline("a\nb", "test.bminor");

    newline.advance();  // consume 'a'
    newline.advance();  // consume '\n'

    auto loc5 = newline.location();

    if (loc5.line != 2 || loc5.column != 1 ||
        loc5.offset != 2 || newline.peek() != 'b') {
        std::cerr << "Test 5 failed!\n";
        return 1;
        }
    std::cout << "Test 5 passed!\n";


    // Test 6: Windows newline
    bminor::SourceCursor windows("a\r\nb", "test.bminor");

    windows.advance();  // consume 'a'
    windows.advance();  // consume '\r'
    windows.advance();  // consume '\n'

    auto loc6 = windows.location();

    if (loc6.line != 2 || loc6.column != 1 ||
        loc6.offset != 3 || windows.peek() != 'b') {
        std::cerr << "Test 6 failed!\n";
        return 1;
        }
    std::cout << "Test 6 passed!\n";


    // Test 7: Empty input
    bminor::SourceCursor empty("", "empty.bminor");

    if (!empty.atEnd() || empty.peek() != '\0' ||
        empty.advance() != '\0' ||
        empty.location().offset != 0) {
        std::cerr << "Test 7 failed!\n";
        return 1;
        }
    std::cout << "Test 7 passed!\n";


    // Test 8: Reading after end of input
    auto endLocation = cursor.location();

    if (cursor.peek() != '\0' ||
        cursor.advance() != '\0' ||
        cursor.location().offset != endLocation.offset) {
        std::cerr << "Test 8 failed!\n";
        return 1;
        }
    
    std::cout << "Test 8 passed!\n";

    std::cout << "All tests passed!\n";
    return 0;
}
