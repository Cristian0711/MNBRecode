using namespace CryptoPP;
class crypto {
private:
	crypto() {};
public:
	[[nodiscard]] static auto get() noexcept -> crypto& {
		static crypto instance;
		return instance;
	}

    auto gen_rsa_key_pair(RSA::PrivateKey& private_key, RSA::PublicKey& public_key) const noexcept -> void
    {
        AutoSeededRandomPool rng;

        private_key.GenerateRandomWithKeySize(rng, 2048);
        public_key.AssignFrom(private_key);
    }

    auto encrypt_rsa_message(const RSA::PublicKey& public_key, const std::string& message, std::string& encrypted_message) const noexcept -> void
    {
        AutoSeededRandomPool rng;

        RSAES_OAEP_SHA_Encryptor encryptor(public_key);
        StringSource ss(message, true, new PK_EncryptorFilter(rng, encryptor, new StringSink(encrypted_message)));
    }

    auto decrypt_rsa_message(const RSA::PrivateKey& private_key, const std::string& encrypted_message, std::string& decrypted_message) const noexcept -> void
    {
        AutoSeededRandomPool rng;

        RSAES_OAEP_SHA_Decryptor decryptor(private_key);
        StringSource ss(encrypted_message, true, new PK_DecryptorFilter(rng, decryptor, new StringSink(decrypted_message)));
    }

    [[nodiscard]] auto get_public_key_in_der_format(const RSA::PublicKey& public_key) const noexcept -> std::string
    {
        std::string der_encoded_key;

        StringSink sink(der_encoded_key);
        public_key.DEREncode(sink);
        sink.MessageEnd();

        return der_encoded_key;
    }

    [[nodiscard]] auto generate_iv(SecByteBlock& iv, size_t size = 16) const noexcept -> std::string
    {
        AutoSeededRandomPool prng;
        prng.GenerateBlock(iv, iv.size());

        std::string buffer;
        ArraySource array(iv, iv.size(), true, new StringSink(buffer));

        // this is for the server to recognise it's an IV for the heartbeat
        if (size != 16)
        {
            buffer += prng.GenerateByte();
            buffer += prng.GenerateByte();
        }

        return buffer;
    }

    [[nodiscard]] auto aes_encrypt(const std::string& message, const std::string& string_key, const SecByteBlock& iv) const noexcept -> std::string
    {
        std::string cipher;

        SecByteBlock key((const byte*)string_key.data(), AES::MAX_KEYLENGTH);

        CBC_Mode< AES >::Encryption e;
        e.SetKeyWithIV(key, key.size(), iv);

        StringSource s(message, true, new StreamTransformationFilter(e, new StringSink(cipher)));

        return cipher;
    }

    [[nodiscard]] auto aes_decrypt(const std::string& message, const std::string& string_key, const std::string& string_iv)const noexcept -> std::string
    {
        std::string decrypted;

        SecByteBlock key((const byte*)string_key.data(), AES::MAX_KEYLENGTH);
        SecByteBlock iv((const byte*)string_iv.data(), AES::BLOCKSIZE);

        CBC_Mode< AES >::Decryption d;
        d.SetKeyWithIV(key, key.size(), iv);

        StringSource s(message, true, new StreamTransformationFilter(d, new StringSink(decrypted)));

        return decrypted;
    }

    [[nodiscard]] auto aes_decrypt(const std::string& message, const std::string& string_key, const SecByteBlock& iv) const noexcept -> std::string
    {
        std::string decrypted;

        SecByteBlock key((const byte*)string_key.data(), AES::MAX_KEYLENGTH);

        CBC_Mode< AES >::Decryption d;
        d.SetKeyWithIV(key, key.size(), iv);

        StringSource s(message, true, new StreamTransformationFilter(d, new StringSink(decrypted)));

        return decrypted;
    }

    [[nodiscard]] auto aes_decrypt_with_iv(const std::string& message, const std::string& string_key) const noexcept -> std::string
    {
        CryptoPP::SecByteBlock iv(reinterpret_cast<const unsigned char*>(message.data()), CryptoPP::AES::BLOCKSIZE);
        return aes_decrypt(message.substr(CryptoPP::AES::BLOCKSIZE), string_key, iv);
    }

    [[nodiscard]] auto aes_encrypt_with_iv(const std::string& message, const std::string& strkey) const noexcept -> std::string
    {
        CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
        return generate_iv(iv) + aes_encrypt(message, strkey, iv);
    }

    [[nodiscard]] auto sha256_encode(const std::string_view message) const noexcept -> std::string
    {
        std::string encrypted_text;
        CryptoPP::SHA256 hash;
        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

        hash.CalculateDigest(digest, (CryptoPP::byte*)message.data(), message.length());

        CryptoPP::HexEncoder sha256_encoder;
        sha256_encoder.Attach(new CryptoPP::StringSink(encrypted_text));
        sha256_encoder.Put(digest, sizeof(digest));
        sha256_encoder.MessageEnd();

        return encrypted_text;
    }

    [[nodiscard]] auto base64_encode(std::string_view message) const noexcept -> std::string
    {
        std::string buffer;

        CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(buffer), false);
        encoder.Put((const CryptoPP::byte*)&message[0], message.size());
        encoder.MessageEnd();

        return buffer;
    }

    [[nodiscard]] auto base64_encode(std::vector<unsigned char>& message) const noexcept -> std::string
    {
        std::string buffer;

        CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(buffer), false);
        encoder.Put((const CryptoPP::byte*)&message[0], message.size());
        encoder.MessageEnd();

        return buffer;
    }

    [[nodiscard]] auto base64_decode(std::string_view message) const noexcept -> std::string
    {
        std::string buffer;

        CryptoPP::Base64Decoder decoder(new CryptoPP::StringSink(buffer));
        decoder.Put((const CryptoPP::byte*)&message[0], message.size());
        decoder.MessageEnd();

        return buffer;
    }

    [[nodiscard]] auto gen_random_string(size_t Size = 32) const noexcept -> std::string
    {
        std::random_device rd;
        std::mt19937 rng(rd());

        std::string alphabet = "1234567890abcdefghijklmnopqrstuvwxyz";
        std::shuffle(std::begin(alphabet), std::end(alphabet), rng);
        alphabet.resize(Size);
        return alphabet;
    }
};