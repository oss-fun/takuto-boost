#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/assert/source_location.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <exception>

// ADR-004: -fno-exceptions / -DBOOST_NO_EXCEPTIONS でビルドする場合、
// Boost 内部の例外送出箇所はユーザー定義の boost::throw_exception に
// フォールバックする。例外情報を std::cerr へダンプした上で
// std::terminate() により即座にアボートさせる。
namespace boost {

void throw_exception(std::exception const& e) {
    std::cerr << "boost::throw_exception: " << e.what() << "\n";
    std::terminate();
}

void throw_exception(std::exception const& e, boost::source_location const& loc) {
    std::cerr << "boost::throw_exception: " << e.what()
              << " at " << loc.file_name() << ":" << loc.line()
              << " (" << loc.function_name() << ")\n";
    std::terminate();
}

}  // namespace boost

int main() {
    const std::string input = "hello boost gzip on wasi\n";

    std::stringstream compressed;

    {
        boost::iostreams::filtering_ostream out;
        out.push(boost::iostreams::gzip_compressor());
        out.push(compressed);
        out << input;
    }

    std::stringstream decompressed;

    {
        boost::iostreams::filtering_istream in;
        in.push(boost::iostreams::gzip_decompressor());
        in.push(compressed);
        boost::iostreams::copy(in, decompressed);
    }

    const std::string result = decompressed.str();

    std::cout << "decompressed: " << result;
    std::cout << "test: " << (result == input ? "OK" : "NG") << "\n";

    return result == input ? 0 : 1;
}
