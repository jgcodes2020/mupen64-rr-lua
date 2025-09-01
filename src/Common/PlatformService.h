/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#define NOMINMAX
#include <Windows.h>

/**
 * \brief A service providing platform-specific functionality.
 */
class PlatformService {
public:
    virtual ~PlatformService() = default;

    virtual std::vector<uint8_t> read_file_buffer(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            return {};
        }

        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            return {};
        }

        return buffer;
    }

    virtual bool write_file_buffer(const std::filesystem::path& path, std::span<uint8_t> data)
    {
        std::ofstream out(path, std::ios::binary);
        if (!out)
        {
            return false;
        }

        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        return out.good();
    }

    virtual std::wstring string_to_wstring(const std::string& str)
    {
#ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        if (size_needed <= 0)
            return L"";

        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
        wstr.pop_back();
        return wstr;
#endif
    }

    virtual std::string wstring_to_string(const std::wstring& wstr)
    {
#ifdef _WIN32
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0)
            return "";

        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
        str.pop_back();
        return str;
#endif
    }


    virtual bool files_are_equal(const std::filesystem::path& first, const std::filesystem::path& second)
    {
        bool different = false;

        FILE* fp1 = nullptr;
        FILE* fp2 = nullptr;
        if (_wfopen_s(&fp1, first.wstring().c_str(), L"rb") || _wfopen_s(&fp2, second.wstring().c_str(), L"rb"))
        {
            return false;
        }
        fseek(fp1, 0, SEEK_END);
        fseek(fp2, 0, SEEK_END);

        const auto len1 = ftell(fp1);
        const auto len2 = ftell(fp2);

        if (len1 != len2)
        {
            different = true;
            goto cleanup;
        }

        fseek(fp1, 0, SEEK_SET);
        fseek(fp2, 0, SEEK_SET);

        int ch1, ch2;
        while ((ch1 = fgetc(fp1)) != EOF && (ch2 = fgetc(fp2)) != EOF)
        {
            if (ch1 != ch2)
            {
                different = true;
                break;
            }
        }

    cleanup:
        fclose(fp1);
        fclose(fp2);
        return !different;
    }


    virtual std::vector<std::wstring> get_files_with_extension_in_directory(std::wstring directory, const std::wstring& extension)
    {
#ifdef _WIN32
        if (directory.empty())
        {
            directory = L".\\";
        }
        else
        {
            if (directory.back() != L'\\')
            {
                directory += L"\\";
            }
        }

        WIN32_FIND_DATA find_file_data;
        const HANDLE h_find = FindFirstFile((directory + L"*." + extension).c_str(), &find_file_data);
        if (h_find == INVALID_HANDLE_VALUE)
        {
            return {};
        }

        std::vector<std::wstring> paths;

        do
        {
            if (!(find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                paths.emplace_back(directory + find_file_data.cFileName);
            }
        }
        while (FindNextFile(h_find, &find_file_data) != 0);

        FindClose(h_find);

        return paths;
#endif
    }

    /**
     * \brief Represents information about path segments.
     */
    struct t_path_segment_info {
        std::wstring drive{};
        std::wstring dir{};
        std::wstring filename{};
        std::wstring ext{};
    };

    /**
     * \brief Gets information about a path's segments.
     * \param path The path to analyze.
     * \param info The structure to fill with path segment information.
     * \return Whether the operation succeeded.
     */
    virtual bool get_path_segment_info(const std::filesystem::path& path, t_path_segment_info& info)
    {
        info.drive = std::wstring(_MAX_DRIVE, 0);
        info.dir = std::wstring(_MAX_DIR, 0);
        info.filename = std::wstring(_MAX_FNAME, 0);
        info.ext = std::wstring(_MAX_EXT, 0);

        const auto result = !(bool)_wsplitpath_s(path.wstring().c_str(), info.drive.data(), info.drive.size(), info.dir.data(), info.dir.size(), info.filename.data(), info.filename.size(), info.ext.data(), info.ext.size());

        if (!result)
        {
            return false;
        }

        auto trim_str = [](std::wstring& str) {
            const size_t null_pos = str.find(L'\0');
            if (null_pos != std::wstring::npos && null_pos > 0)
            {
                str.resize(null_pos);
            }
            else if (null_pos == 0)
            {
                str.clear();
            }
        };

        trim_str(info.drive);
        trim_str(info.dir);
        trim_str(info.filename);
        trim_str(info.ext);

        return true;
    }
};
