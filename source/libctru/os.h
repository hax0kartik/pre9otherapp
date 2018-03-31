/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once
#include "svc.h"

/// Packs a system version from its components.
#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

/// Retrieves the major version from a packed system version.
#define GET_VERSION_MAJOR(version)    ((version) >>24)

/// Retrieves the minor version from a packed system version.
#define GET_VERSION_MINOR(version)    (((version)>>16)&0xFF)

/// Retrieves the revision version from a packed system version.
#define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

/// Memory regions.
typedef enum
{
	MEMREGION_ALL = 0,         ///< All regions.
	MEMREGION_APPLICATION = 1, ///< APPLICATION memory.
	MEMREGION_SYSTEM = 2,      ///< SYSTEM memory.
	MEMREGION_BASE = 3,        ///< BASE memory.
} MemRegion;

/// Tick counter.
typedef struct
{
	u64 elapsed;   ///< Elapsed CPU ticks between measurements.
	u64 reference; ///< Point in time used as reference.
} TickCounter;

/// OS_VersionBin. Format of the system version: "<major>.<minor>.<build>-<nupver><region>"
typedef struct
{
	u8 build;
	u8 minor;
	u8 mainver;//"major" in CVER, NUP version in NVer.
	u8 reserved_x3;
	char region;//"ASCII character for the system version region"
	u8 reserved_x5[0x3];
} OS_VersionBin;

/**
 * @brief Converts an address from virtual (process) memory to physical memory.
 * @param vaddr Input virtual address.
 * @return The corresponding physical address.
 * It is sometimes required by services or when using the GPU command buffer.
 */
u32 osConvertVirtToPhys(const void* vaddr);

/**
 * @brief Converts 0x14* vmem to 0x30*.
 * @param vaddr Input virtual address.
 * @return The corresponding address in the 0x30* range, the input address if it's already within the new vmem, or 0 if it's outside of both ranges.
 */
void* osConvertOldLINEARMemToNew(const void* vaddr);

/**
 * @brief Retrieves basic information about a service error.
 * @param error Error to retrieve information about.
 * @return A string containing a summary of an error.
 *
 * This can be used to get some details about an error returned by a service call.
 */
const char* osStrError(u32 error);