class InvalidBase64Exception
{
};

string r_base64_encode (const char* from, size_t length) throw(InvalidBase64Exception);
string r_base64_decode (const char* from, size_t length) throw(InvalidBase64Exception);
inline string r_base64_decode (string const& from) throw(InvalidBase64Exception)
{
    return r_base64_decode(from.c_str(), from.length());
}

