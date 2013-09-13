// auto_build_version.h

#if !defined(XFS_SHARE_XFS_INTERNAL_XFS_BUILD_VERSION_H_)
#define XFS_SHARE_XFS_INTERNAL_XFS_VERSION_H_

_START_XFS_BASE_NAMESPACE_

const char* GetXFSRevisionNum();

const char* GetXFSBuildInfo();

// xfs version: " ${revision_num} ", xfs-internal-uuid: D5FE64F8-6AC5-4254-8B5A-EED3C9FC72FE
const char* GetXFSVerAndUUID();

const char* GetXFSReleaseVer();

_END_XFS_BASE_NAMESPACE_

#endif //XFS_SHARE_XFS_INTERNAL_XFS_BUILD_VERSION_H_

