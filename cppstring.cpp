#include <string>
#include <vector>
#include <iostream>
using namespace std;

int main()
{
    char fill_char{'.'};
    string pyramid_base{};
    cout << "Enter the base of the pyramid: ";
    getline(cin, pyramid_base);

    size_t pyramid_base_length{pyramid_base.length()};
    size_t pyramid_base_level_length{(pyramid_base_length*2)-1};

    vector<string> pyramid_levels{};
    for (size_t i{0}; i < pyramid_base_length; ++i)
    {
        size_t pyramid_level_length = ((i+1)*2)-1;
        size_t space_fill_length = (pyramid_base_level_length - pyramid_level_length) / 2;

        string pyramid_level{};
        for (size_t f{0}; f < space_fill_length; ++f) pyramid_level += fill_char;
        for (size_t j{0}; j <= (i); ++j) pyramid_level += pyramid_base[j];
        for (size_t j{pyramid_level.length() - 2}; j >= space_fill_length && j < pyramid_level.length(); --j) pyramid_level += pyramid_level[j];
        for (size_t f{0}; f < space_fill_length; ++f) pyramid_level += fill_char;
    
        pyramid_levels.push_back(pyramid_level);
    }
    
    for (size_t i{0}; i < pyramid_levels.size(); ++i)
    {
        cout << pyramid_levels[i] << endl;
    }
    return 0;
}
