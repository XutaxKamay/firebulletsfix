#ifndef PATTERNSCAN_H
#define PATTERNSCAN_H

#include "main.h"

unsigned char hex_digit_to_byte(char c);
int hex_char_to_byte(char c1, char c2);
void swap_endian(unsigned char* addr, size_t len);

class CPatternScan
{
 public:
    CPatternScan()
    {}

    CPatternScan(std::string strModule, std::string strPattern) :
        m_strPattern(strPattern), m_strModule(strModule), m_ptrFound(nullptr)
    {
        // Msg("Init: %s\n", m_strPattern.c_str());
        findCode();
    }

#ifndef _WINDLL
    static int getCodeStartAddr(dl_phdr_info* info,
                                size_t size,
                                ptr_t moduleBase)
    {
        ptr_t* pModuleBase = reinterpret_cast<ptr_t*>(moduleBase);

        if (info->dlpi_addr == reinterpret_cast<uintptr_t>(*pModuleBase))
        {
            for (int i = 0; i < info->dlpi_phnum; i++)
            {
                // Probably code section.
                if (info->dlpi_phdr[i].p_flags & PF_X)
                {
                    *reinterpret_cast<uintptr_t*>(pModuleBase) =
                        info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;

                    goto end;
                }
            }

            *pModuleBase = nullptr;
        }

    end:
        return 0;
    }

    static int getCodeEndAddr(dl_phdr_info* info, size_t size, ptr_t moduleBase)
    {
        ptr_t* pModuleBase = reinterpret_cast<ptr_t*>(moduleBase);

        if (info->dlpi_addr == reinterpret_cast<uintptr_t>(*pModuleBase))
        {
            for (int i = 0; i < info->dlpi_phnum; i++)
            {
                // Probably code section.
                if (info->dlpi_phdr[i].p_flags & PF_X)
                {
                    *reinterpret_cast<uintptr_t*>(
                        pModuleBase) = info->dlpi_addr +
                                       info->dlpi_phdr[i].p_vaddr +
                                       info->dlpi_phdr[i].p_memsz;

                    goto end;
                }
            }

            *pModuleBase = nullptr;
        }

    end:
        return 0;
    }

#endif

    void findCode()
    {
#ifndef _WINDLL
        auto lm = reinterpret_cast<link_map*>(
            dlopen(m_strModule.c_str(), RTLD_NOW));

        if (lm == nullptr)
        {
            Msg("Couldn't find module\n");
            return;
        }

        uintptr_t startAddr = *reinterpret_cast<uintptr_t*>(&lm->l_addr);
        dl_iterate_phdr(getCodeStartAddr, &startAddr);

        if (startAddr == 0)
        {
            Msg("Couldn't find start addr\n");
            return;
        }

        uintptr_t endAddr = *reinterpret_cast<uintptr_t*>(&lm->l_addr);
        dl_iterate_phdr(getCodeEndAddr, &endAddr);

        if (endAddr == 0)
        {
            Msg("Couldn't find end addr\n");
            return;
        }
#else

        auto startAddr = reinterpret_cast<uintptr_t>(
            GetModuleHandleA(m_strModule.c_str()));

        auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(startAddr);

        auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uintptr_t>(dosHeader) + dosHeader->e_lfanew);

        auto endAddr = startAddr + *reinterpret_cast<uintptr_t*>(
                                       &ntHeaders->OptionalHeader.SizeOfImage);
#endif

        auto scanMem = startAddr;

        while (m_ptrFound == nullptr)
        {
            auto byte = reinterpret_cast<unsigned char*>(scanMem);

            if (reinterpret_cast<uintptr_t>(byte) > endAddr)
            {
                Msg("Couldn't find pattern: %s\n", m_strPattern.c_str());
                assert(0);
            }

            for (uintptr_t patternDelta = 0;
                 patternDelta < m_strPattern.size();)
            {
                auto byteStr = reinterpret_cast<char*>(
                    reinterpret_cast<uintptr_t>(m_strPattern.data()) +
                    patternDelta);

                // If it's an unknown byte, we skip the byte.
                if (*byteStr == '?')
                {
                    patternDelta += 2;
                    byte++;
                    continue;
                }

                auto patternByte = hex_char_to_byte(*byteStr, *(byteStr + 1));

                if (patternByte == 0x100)
                {
                    Msg("Wrong pattern!!! %s\n", m_strPattern.c_str());
                    return;
                }

                if (patternByte != static_cast<int>(*byte))
                    goto dontmatch;

                patternDelta += 3;
                byte++;
            }

            Msg("Found pattern (%p): %s\n", scanMem, m_strPattern.c_str());
            m_ptrFound = reinterpret_cast<ptr_t>(scanMem);

        dontmatch:

            scanMem++;
        }
    }

    const std::string& getStr()
    {
        return m_strPattern;
    }

    template <typename T = ptr_t>
    const T get()
    {
        return reinterpret_cast<T>(m_ptrFound);
    }

 private:
    std::string m_strPattern, m_strModule;
    ptr_t m_ptrFound;
};

#endif