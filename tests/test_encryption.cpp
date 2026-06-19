#include "test_framework.h"
#include "../include/data/encryption.h"
#include <string>

using namespace opep;

// Kiem tra xorCipher la phep doi xung: xor(xor(data)) == data
TEST_CASE(test_enc_xorCipher_symmetric) {
    std::string original = "Hello World 123!";
    std::string encrypted = xorCipher(original);
    std::string decrypted = xorCipher(encrypted);
    ASSERT_EQ(original, decrypted);
}

// Kiem tra xorCipher voi chuoi rong tra ve chuoi rong
TEST_CASE(test_enc_xorCipher_empty_string) {
    ASSERT_EQ(xorCipher(""), std::string(""));
}

// Kiem tra xorCipher voi key tuy chinh van doi xung
TEST_CASE(test_enc_xorCipher_custom_key) {
    std::string key  = "MYKEY";
    std::string data = "Secret Data 999";
    ASSERT_EQ(data, xorCipher(xorCipher(data, key), key));
}

// Kiem tra xorCipher voi key 1 ky tu
TEST_CASE(test_enc_xorCipher_single_char_key) {
    std::string data = "ABCDEFGH";
    ASSERT_EQ(data, xorCipher(xorCipher(data, "X"), "X"));
}

// Kiem tra hexEncode tao ra chuoi hex hop le (chi co 0-9, a-f)
TEST_CASE(test_enc_hexEncode_valid_chars) {
    std::string encoded = hexEncode("Hello World");
    for (char c : encoded) {
        bool valid = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        ASSERT_TRUE(valid);
    }
}

// Kiem tra hexEncode do dai: moi byte → 2 ky tu hex
TEST_CASE(test_enc_hexEncode_length) {
    std::string data = "ABC"; // 3 bytes
    ASSERT_EQ(hexEncode(data).size(), std::size_t(6));
}

// Kiem tra hexEncode → hexDecode phuc hoi dung chuoi goc
TEST_CASE(test_enc_hex_roundtrip_ascii) {
    std::string original = "Test data 123 !@#$%";
    ASSERT_EQ(original, hexDecode(hexEncode(original)));
}

// Kiem tra hexEncode → hexDecode voi byte cao (non-ASCII)
TEST_CASE(test_enc_hex_roundtrip_binary) {
    std::string original;
    original.push_back('\x00');
    original.push_back('\x7F');
    original.push_back(static_cast<char>('\x80'));
    original.push_back(static_cast<char>('\xFF'));
    ASSERT_EQ(original, hexDecode(hexEncode(original)));
}

// Kiem tra hexDecode voi chuoi rong tra ve chuoi rong
TEST_CASE(test_enc_hexDecode_empty) {
    ASSERT_EQ(hexDecode(""), std::string(""));
}

// Kiem tra encryptToFile → decryptFromFile phuc hoi dung plain text
TEST_CASE(test_enc_file_roundtrip) {
    std::string data = "Line1\nLine2\nLine3\n---OPEP---\n";
    ASSERT_EQ(data, decryptFromFile(encryptToFile(data)));
}

// Kiem tra encryptToFile output chi chua ky tu hex an toan (0-9, a-f)
TEST_CASE(test_enc_encryptToFile_printable) {
    std::string enc = encryptToFile("Sensitive data 42");
    for (char c : enc) {
        bool safe = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        ASSERT_TRUE(safe);
    }
}

// Kiem tra encryptToFile voi key khac nhau
TEST_CASE(test_enc_file_custom_key_roundtrip) {
    std::string data = "Secret info";
    std::string key  = "CUSTOM_KEY_123";
    ASSERT_EQ(data, decryptFromFile(encryptToFile(data, key), key));
}

// Kiem tra hashPassword: cung input → cung output (deterministic)
TEST_CASE(test_enc_hashPassword_consistent) {
    std::string h1 = hashPassword("myPassword123");
    std::string h2 = hashPassword("myPassword123");
    ASSERT_EQ(h1, h2);
}

// Kiem tra hashPassword: khac input → khac output
TEST_CASE(test_enc_hashPassword_different_passwords) {
    ASSERT_NE(hashPassword("password1"), hashPassword("password2"));
    ASSERT_NE(hashPassword("abc123"),    hashPassword("ABC123"));
}

// Kiem tra hashPassword khong tra ve plain text (one-way)
TEST_CASE(test_enc_hashPassword_not_reversible) {
    std::string pw   = "mySecret";
    std::string hash = hashPassword(pw);
    // Hash khac plain text
    ASSERT_NE(hash, pw);
    // Hash la chuoi hex printable
    for (char c : hash) {
        bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        ASSERT_TRUE(hex);
    }
}

// --- Ham chay toan bo Encryption tests ---
void runEncryptionTests() {
    std::cout << "\n--- Encryption Tests ---\n";
    RUN_TEST(test_enc_xorCipher_symmetric);
    RUN_TEST(test_enc_xorCipher_empty_string);
    RUN_TEST(test_enc_xorCipher_custom_key);
    RUN_TEST(test_enc_xorCipher_single_char_key);
    RUN_TEST(test_enc_hexEncode_valid_chars);
    RUN_TEST(test_enc_hexEncode_length);
    RUN_TEST(test_enc_hex_roundtrip_ascii);
    RUN_TEST(test_enc_hex_roundtrip_binary);
    RUN_TEST(test_enc_hexDecode_empty);
    RUN_TEST(test_enc_file_roundtrip);
    RUN_TEST(test_enc_encryptToFile_printable);
    RUN_TEST(test_enc_file_custom_key_roundtrip);
    RUN_TEST(test_enc_hashPassword_consistent);
    RUN_TEST(test_enc_hashPassword_different_passwords);
    RUN_TEST(test_enc_hashPassword_not_reversible);
}
