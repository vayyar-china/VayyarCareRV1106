#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <ctime>
#include <chrono>
#include <vector>
#include <iomanip>

#include <openssl/evp.h>
#include <openssl/provider.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/asn1.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/hmac.h>

using namespace std::chrono_literals;

constexpr auto HTTP_REQ_JWT_EXPIRY = 150min;
#define TOKEN "-----BEGIN RSA PRIVATE KEY-----\n"                                  \
              "MIIEpAIBAAKCAQEAol2+sYQPHCEUzOiMkPwkpvOAMELjKGd7tUh5six+INLtddkM\n" \
              "8bg1UfioRRH2guk2cNq1+xhj1nsN1bPUK+2gL0rz1715ld/BtFwn4TDCk4NDMpRv\n" \
              "BFTL7l+SL+MHEwcllP+ELVxfbKXSJmSQ4vX7dDfX+Tr83AQ6D3rEfx6zltrHaQvk\n" \
              "p7tM6QxzZ4nxaxt/muusP/ziN/VKZL2bZRVvWtHStaXH+cg8j9Y+ziq8PO4jn2gn\n" \
              "s0P2hUewfLOcAB9NeoQztMIyLdZVq8VbEIt5M6WhZqguzvkoAdxtGSvlp/9RbMdC\n" \
              "jcHeNrSoGolfFIFINdC6oib0i1Uv92DzfM32swIDAQABAoIBAG+4HUw718G9KXjM\n" \
              "0lu4gue1SJDslSzp938HPWVRo/+l4uphzLxNybJ7bB0KfRoaqfEBLJJ3d8rOpPhU\n" \
              "YjtPqBaidiIOJ6jua3RUrMQQLvIdMZGN/M14I1IDxKzag7WeB0f8gNBNbQNvHah5\n" \
              "LhBcvQZ8nkjJQ8+HwQOBr87gKyYyjpBOa9GQtegmKSDQkgUfNtQFtJM9H0MT9eVP\n" \
              "qPLIwkrahY7G7eiIwHioslvyerqT8VjWbJi6qnPGLzA6tULFCfuOtbQkFsFrltlC\n" \
              "YspTr9cqiZcQdKWXs+DCi43XGPpFrrGXbjQUTPuoAVC0c0D5AaRacGcDWieu8kSJ\n" \
              "X2GJiZECgYEA1ILyn77uIjvhLLngvc2hAswA+on99MTipI6qTKElfRSa+xlEdGA7\n" \
              "me34xzmGafWks2/xK2mh4I/RUsuUSm9YNjl4iZ+0PTs40wcRM87nEY3f+s5vlRg6\n" \
              "YWLOBsqufCpOb7A82kivhAhnEfylaDi/A0SHh82WrTd3h0u9Duwh4TcCgYEAw5fI\n" \
              "vtc463PATV8pImpd6N6DpC7Q6wTC3l9KsPm5H3RLtWqZ6Ut2t1tWocHfJ4BwfDpG\n" \
              "cFHP8Ciewh2/SfMuf9U8HclhgZrHPw8nAr/CqZkzc5sVAdGnAONB986Du0zxuCaT\n" \
              "VS+5Iyuyh6LkrglpfQkD9QtthGgUyHnjhX4OxGUCgYEApPwE4YCZ4Hocl1y33qOG\n" \
              "HWXEXL5FK4KvcpJQJK33LOSSbd9wBemwXBEk04cVk63h8G04DzwzMOR23pCu0Bsh\n" \
              "oPd9XVtf0ynnM+6IrHA1dKtw+IbMmjP1HyZWTW+Nh1hRDOUGWXGU0iyz2IMM550m\n" \
              "rqLFlelG0bP+WEu0u6EXrIECgYBo86M2NApI3R2M5skNvIGTmQKOMMjNswpVhFBx\n" \
              "0i3xoNeXCxJ3SpTzIkHEHmF9sr+pCSQO1Pd86G73vjqMlb6XvXW6jfqSLtlHDvxh\n" \
              "zz/G/XvMRqTpCvRP16HoNJofANSbeaeJCEiRhLsRaOjIMii2fQXwkSc86xRr1mut\n" \
              "j8yguQKBgQDJ1NtRIPhV6rr0hkyBR3uDTc9qLmAREWcz46JCHwk7rmcphxHN+AnF\n" \
              "h4L40lBqc3RHRxiKaMg2PtI/tT+kdaPPgjyH5du9zW3m9ppfRK+LXmsYXa6rR2C5\n" \
              "vyp5g5GxKzJEMn6lh8ygIm1tbZ1iHeHDKirbhonOaaZR2xDO6gj8CQ==\n"         \
              "-----END RSA PRIVATE KEY-----"

void init_openssl();

struct EVP_PKEY_Deleter { void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); } };
struct EVP_MD_CTX_Deleter { void operator()(EVP_MD_CTX* m) const { EVP_MD_CTX_free(m); } };
struct EVP_PKEY_CTX_Deleter { void operator()(EVP_PKEY_CTX* c) const { EVP_PKEY_CTX_free(c); } };
struct X509_Deleter { void operator()(X509* x) const { X509_free(x); } };
struct X509_NAME_Deleter { void operator()(X509_NAME* n) const { X509_NAME_free(n); } };
struct BIO_Deleter { void operator()(BIO* b) const { BIO_free(b); } };
struct OSSL_PROVIDER_Deleter { void operator()(OSSL_PROVIDER* prov) const { OSSL_PROVIDER_unload(prov); } };
struct ASN1_INTEGER_Deleter { void operator()(ASN1_INTEGER* a) const { ASN1_INTEGER_free(a); } };

std::string base64_url_encode(const std::vector<uint8_t>& data);
std::string base64_url_encode(const std::string& data);

std::string create_jwt(const uint8_t *private_key, size_t private_key_size,
                    const std::string &registry_id, const std::string & project_id, const std::string& device_id,
                    std::chrono::seconds jwt_expiry_time);

bool generate_keys(std::string &local_pk_pem, std::string & local_x509_pem);

std::string generate_hmac(const std::string &api_secret, const std::string &password);