#include "Auth.h"

void init_openssl()
{
    setenv("OPENSSL_CONF", "./ssl/openssl.cnf", 1);
    // OPENSSL_init_ssl(OPENSSL_INIT_NO_LOAD_CONFIG, nullptr);
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

}

std::string base64_url_encode(const std::vector<uint8_t>& data)
{
    // create BIO chain
    BIO *bio, *b64, *bio_out;
    b64 = BIO_new(BIO_f_base64());
    if (!b64) return "";

    // 设置无换行模式
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    bio_out = BIO_new(BIO_s_mem());
    if (!bio_out) {
        BIO_free(b64);
        return "";
    }
    
    // 将 b64 和 bio_out 连接起来
    bio = BIO_push(b64, bio_out);
    
    // 写入数据
    if (BIO_write(bio, data.data(), static_cast<int>(data.size())) <= 0) {
        BIO_free_all(bio);
        return "";
    }
    
    // 刷新 BIO
    if (BIO_flush(bio) != 1) {
        BIO_free_all(bio);
        return "";
    }
    
    // 获取编码后的数据
    char* out_data;
    long out_len = BIO_get_mem_data(bio_out, &out_data);
    std::string result(out_data, out_len);
    
    // 释放 BIO
    BIO_free_all(bio);
    
    // URL 安全替换
    for (char& c : result) {
        if (c == '+') c = '-';
        if (c == '/') c = '_';
    }
    
    // 移除填充（=）
    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }
    
    return result;
}

std::string base64_url_encode(const std::string& data)
{
    std::vector<uint8_t> byte_data(data.begin(), data.end());
    return base64_url_encode(byte_data);
}

std::string create_jwt(const uint8_t *private_key, size_t private_key_size,
                    const std::string &registry_id, const std::string & project_id, const std::string& device_id,
                    std::chrono::seconds jwt_expiry_time)
{
    std::unique_ptr<OSSL_PROVIDER, OSSL_PROVIDER_Deleter> default_provider(OSSL_PROVIDER_load(nullptr, "default"));
    std::unique_ptr<OSSL_PROVIDER, OSSL_PROVIDER_Deleter> legacy_provider(OSSL_PROVIDER_load(nullptr, "legacy"));

    // JWT header;
    std::string header = "{\"alg\":\"RS256\",\"type\":\"JWT\",\"deviceId\":\"" + 
                        device_id + "\",\"registryId\":\"" + registry_id + "\"}";

    // Get current and expire time
    auto now = std::chrono::system_clock::now();
    auto iat = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    auto exp = iat + jwt_expiry_time;

    // JWT payload
    std::string payload = "{\"iat\":" + std::to_string(iat.count()) + 
                        ",\"exp\":" + std::to_string(exp.count()) + 
                        ",\"aud\":\"" + project_id + "\"}";

    // base64 url encode header and payload
    std::string header_b64 = base64_url_encode(header);
    std::string payload_b64 = base64_url_encode(payload);

    // compose header & payload
    std::string header_payload = header_b64 + "." + payload_b64;

    std::unique_ptr<BIO, BIO_Deleter> bio(BIO_new_mem_buf(private_key, private_key_size));
    if (!bio)
    {
        std::cerr << "Failed to create BIO for private key" << std::endl;
        return "";
    }

    // parse private key
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pkey(PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr));
    if (!pkey)
    {
        std::cerr << "Failed to parse private key: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    // create message digest context
    std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> md_ctx(EVP_MD_CTX_new());
    if (!md_ctx)
    {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return "";
    }

    // initialize sign
    if (EVP_DigestSignInit(md_ctx.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) != 1)
    {
        std::cerr << "Failed to initialize signing: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    // update sign
    if (EVP_DigestSignUpdate(md_ctx.get(), header_payload.data(), header_payload.size()) != 1)
    {
        std::cerr << "Failed to update signing data: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    // size of sign
    size_t signature_len;
    if (EVP_DigestSignFinal(md_ctx.get(), nullptr, &signature_len) != 1)
    {
        std::cerr << "Failed to determine signature length: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    // generate sign
    std::vector<unsigned char> signature(signature_len);
    if (EVP_DigestSignFinal(md_ctx.get(), signature.data(), &signature_len) != 1)
    {
        std::cerr << "Failed to generate signature: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    signature.resize(signature_len);

    // base64 encode sign
    std::string signature_str(reinterpret_cast<char*>(signature.data()), signature.size());
    std::string signature_b64 = base64_url_encode(signature_str);

    return header_payload + "." + signature_b64;
}

bool generate_keys(std::string &local_pk_pem, std::string & local_x509_pem)
{
    std::unique_ptr<OSSL_PROVIDER, OSSL_PROVIDER_Deleter> default_provider(OSSL_PROVIDER_load(nullptr, "default"));
    if (!default_provider)
    {
        std::cerr << "Failed to load default provider" << std::endl;
        return false;
    }

    std::unique_ptr<OSSL_PROVIDER, OSSL_PROVIDER_Deleter> legacy_provider(OSSL_PROVIDER_load(nullptr, "legacy"));
    if (!legacy_provider)
    {
        std::cerr << "Failed to load legacy provider" << std::endl;

        // EVP_set_default_properties(nullptr, "provider=default");
        // EVP_add_alg_module();
        // return false;
    }

    // create key generator context
    std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_Deleter> ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    if (!ctx)
    {
        std::cerr << "Failed to create EVP_PKEY_CTX" << std::endl;
        return false;
    }

    // initialize key generator
    if (EVP_PKEY_keygen_init(ctx.get()) <= 0)
    {
        std::cerr << "Failed to initialize key generation" << std::endl;
        return false;
    }

    // set parameters
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) <= 0)
    {
        std::cerr << "Failed to set RSA key length" << std::endl;
        return false;
    }

    // Generate private key
    std::cout << "Auth: generating PK ..." << std::endl;
    EVP_PKEY* pkey_ptr = nullptr;
    if (EVP_PKEY_generate(ctx.get(), &pkey_ptr) <= 0)
    {
        std::cerr << "Failed to generate RSA key" << std::endl;
        return false;
    }

    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pkey(pkey_ptr);

    std::unique_ptr<BIO, BIO_Deleter> bio_priv(BIO_new(BIO_s_mem()));
    if (PEM_write_bio_PrivateKey(bio_priv.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr) != 1)
    {
        std::cerr << "Failed to write private key PEM" << std::endl;
        return false;
    }

    // get PEM string
    char* priv_data = nullptr;
    long priv_len = BIO_get_mem_data(bio_priv.get(), &priv_data);
    local_pk_pem = std::string(priv_data, priv_len);
    std::cout << "Auth: generated PK pem\n" << local_pk_pem << "\n (" << priv_len << "B)" << std::endl;

    // generate signature
    std::cout << "Auth: generating x509 ..." << std::endl;
    std::unique_ptr<X509, X509_Deleter> x509(X509_new());
    if (!x509)
    {
        std::cerr << "Failed to create x509 certificate" << std::endl;
        return false;
    }

    // set certificate version
    X509_set_version(x509.get(), 2);

    // random serial
    unsigned char serial_bytes[20];
    if (RAND_bytes(serial_bytes, sizeof(serial_bytes)) != 1)
    {
        std::cerr << "Failed to generate random serial" << std::endl;
        return false;
    }

    // ensure serial positive
    serial_bytes[0] &= 0x7F;

    // convert serial to BIGNUM
    BIGNUM* bn_serial = BN_bin2bn(serial_bytes, sizeof(serial_bytes), nullptr);
    if (!bn_serial)
    {
        std::cerr << "Failed to create serial BIGNUM" << std::endl;
        return false;
    }

    // set certificate serial
    std::cout << "Set certificate serial" << std::endl;
    std::unique_ptr<ASN1_INTEGER, ASN1_INTEGER_Deleter> serial(ASN1_INTEGER_new());
    if (!serial || !BN_to_ASN1_INTEGER(bn_serial, serial.get()))
    {
        std::cerr << "Failed to set certificate serial" << std::endl;
        BN_free(bn_serial);
        return false;
    }
    BN_free(bn_serial);

    X509_set_serialNumber(x509.get(), serial.get());

    // set expire date
    std::cout << "config expire date" << std::endl;
    if (X509_gmtime_adj(X509_getm_notBefore(x509.get()), 0) == nullptr)
    {
        std::cerr << "Failed to set notBefore time" << std::endl;
        return false;
    }
  
    if (X509_gmtime_adj(X509_getm_notAfter(x509.get()), 60 * 60 * 24 * 365 * 30) == nullptr)
    {
        std::cerr << "Failed to set notAfter time" << std::endl;
        return false;
    }

    // time_t now = time(nullptr);
    // struct tm tm_expire = *localtime(&now);
    // tm_expire.tm_year += 80;
    // time_t expire = mktime(&tm_expire);

    // X509_gmtime_adj(X509_getm_notBefore(x509.get()), 0);
    // X509_gmtime_adj(X509_getm_notAfter(x509.get()), expire - now);

    // std::cout << "Not before " << now << ", not after " << expire << std::endl;

    // set subject
    std::cout << "x509 settings and options" << std::endl;
    std::unique_ptr<X509_NAME, X509_NAME_Deleter> name(X509_NAME_new());
    if (!name || X509_NAME_add_entry_by_txt(name.get(), "CN", MBSTRING_ASC,
                                            (const unsigned char*)"home.walabot.com", -1, -1, 0) != 1)
    {
        std::cerr << "Failed to set subject name" << std::endl;
        return false;
    }
    std::cout << "set subject name" << std::endl;
    X509_set_subject_name(x509.get(), name.get());
    std::cout << "set issuer name" << std::endl;
    X509_set_issuer_name(x509.get(), name.get());
    std::cout << "set pubkey" << std::endl;
    if (X509_set_pubkey(x509.get(), pkey.get())!= 1)
    {
        std::cerr << "Failed to set public key" << std::endl;
        return false;
    }

    // add extension opt
    X509V3_CTX ctx_v3;
    X509V3_set_ctx_nodb(&ctx_v3);
    X509V3_set_ctx(&ctx_v3, x509.get(), x509.get(), nullptr, nullptr, 0);
    
    // add subject identifier
    std::cout << "add subect identifier" << std::endl;
    std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> sk_ext(
        X509V3_EXT_conf_nid(nullptr, &ctx_v3, NID_subject_key_identifier, "hash"), X509_EXTENSION_free
    );
    if (!sk_ext || !X509_add_ext(x509.get(), sk_ext.get(), -1))
    {
        std::cerr << "Failed to add subject key identifier" << std::endl;
        return false;
    }

    // add issuer identifier
    std::cout << "add issuer identifier" << std::endl;
    std::unique_ptr<X509_EXTENSION, decltype(&X509_EXTENSION_free)> ak_ext(
        X509V3_EXT_conf_nid(nullptr, &ctx_v3, NID_authority_key_identifier, "keyid:always"), X509_EXTENSION_free
    );
    if (!ak_ext || !X509_add_ext(x509.get(), ak_ext.get(), -1))
    {
        std::cerr << "Failed to add authroity key identifier" << std::endl;
        return false;
    }

    // use pk to sign certificate
    if (X509_sign(x509.get(), pkey.get(), EVP_sha256()) <= 0)
    {
        std::cerr << "Failed to sign certificate: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return false;
    }

    // output certificate as PEM
    std::unique_ptr<BIO, BIO_Deleter> bio_cert(BIO_new(BIO_s_mem()));
    if (PEM_write_bio_X509(bio_cert.get(), x509.get()) != 1)
    {
        std::cerr << "Failed to write certificate PEM" << std::endl;
        return false;
    }

    char* cert_data = nullptr;
    long cert_len = BIO_get_mem_data(bio_cert.get(), &cert_data);
    local_x509_pem = std::string(cert_data, cert_len);
    std::cout << "Auth: generated x509 pem\n" << local_x509_pem << "\n (" << cert_len << "B)" << std::endl;

    return true;
}

std::string generate_hmac(const std::string &api_secret, const std::string &password)
{
    std::cout << "[Aliyun][secret]=" << api_secret << std::endl;
    std::cout << "[Aliyun][password]=" << password << std::endl;

    unsigned char hmac_result[EVP_MAX_MD_SIZE] = {'\0'};
    unsigned int hmac_len = 0;

    HMAC(EVP_sha256(),
        api_secret.c_str(), api_secret.length(),
        reinterpret_cast<const unsigned char*>(password.c_str()), password.length(),
        hmac_result, &hmac_len);

    // convert binary to HEX format
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hmac_len; ++i)
        ss << std::setw(2) << static_cast<unsigned int>(hmac_result[i]);

    return ss.str();
}