#include "z64rom.h"
#include "tools.h"
#include <ext_zip.h>

// update channels live here, in case user ever needs to change them
static const char *updateChannelFilename = "update-channel.txt";

const char* sTools[] = {
#ifdef _WIN32
	"tools/mips64-binutils/bin/mips64-gcc.exe",
	"tools/mips64-binutils/bin/mips64-ld.exe",
	"tools/mips64-binutils/bin/mips64-objdump.exe",
	"tools/mips64-binutils/bin/mips64-objcopy.exe",
	"tools/z64audio.exe",
	"tools/z64convert.exe",
	"tools/z64yartool.exe",
	"tools/z64playas.exe",
	"tools/seq64_console.exe",
	"tools/seqas.exe",
	"tools/novl.exe",
#else
	"tools/mips64-binutils/bin/mips64-gcc",
	"tools/mips64-binutils/bin/mips64-ld",
	"tools/mips64-binutils/bin/mips64-objdump",
	"tools/mips64-binutils/bin/mips64-objcopy",
	"tools/z64audio",
	"tools/z64convert",
	"tools/z64yartool",
	"tools/z64playas",
	"tools/seq64_console",
	"tools/seqas",
	"tools/novl",
	"nm",
#endif
};

static s32 ZipProgressCallback(const char* name, f32 prcnt) {
	info_prog(gLang.setup.extracting, (u32)prcnt, 100);
	
	return 0;
}

/*============================================================================*/

static bool Tools_ValidateTools_Additional(void) {
	for (int i = 0; i < ArrCount(sTools); i++) {
		const char *name = sTools[i];
#ifndef _WIN32
		// if the tool is a linux command, ensure it is available
		if (!strchr(name, '/'))
		{
			if (system(x_fmt("which %s > /dev/null 2>&1", name)) != 0)
				errr("'%s' command is not available on your linux system", name);
			
			continue;
		}
#endif
		if (!sys_stat(name))
			return 1 + printf("tool '%s' was not found\n", name);
	}
	
	if (!sys_stat("include/z64hdr/common/z64common.h"))
		return 1;
	
	return 0;
}

static s32 Tools_ValidateTools_Required(void) {
	u32 fail = 0;
	
	const char* toolList[] = {
		"include/sequence.inc",
		"include/ulib_linker.ld",
		
		"tools/actor-object-deb.toml",
		"tools/z64audio.cfg",
		
#ifdef _WIN32
		"tools/novl.exe",
		"tools/z64audio.exe",
		"tools/z64convert.exe",
		"tools/seq64_console.exe",
		"tools/seqas.exe",
#else
		"tools/novl",
		"tools/z64audio",
		"tools/z64convert",
		"tools/seq64_console",
		"tools/seqas",
#endif
	};
	
	for (int i = 0; i < ArrCount(toolList); i++) {
		if (!sys_stat(toolList[i])) {
			fail = true;
			warn_align(gLang.err_missing, "%s", toolList[i]);
		}
	}
	
	if (fail) {
		printf("\n");
		warn(gLang.setup.err_missing_components,
			"https://github.com/z64tools/z64rom/releases");
	}
	
	return fail;
}

static void Tools_CheckUpdateImpl() {
	char buffer[1024];
	char* tag;
	u32 sz = 0;
	u32 vnum[3] = { -1, -1, -1 };
	u32 cnum[3] = { -1, -1, -1 };
	u32 curVer, newVer;
	Memfile api = Memfile_New();
	Memfile updateChannel = Memfile_New();
	
	// load from update channel
	Memfile_LoadStr(&updateChannel, updateChannelFilename);
	char *url = updateChannel.str;
	url[strcspn(url, "\r\n")] = '\0'; // want only the first line
	
	if (Memfile_Download(&api, url, NULL))
		goto error;
	
	tag = strstr(api.str, "\"tag_name\":");
	if (tag == NULL) goto error;
	tag += strlen("\"tag_name\":\"");
	while (tag[sz] != '\"') sz++;
	
	memset(buffer, 0, sz + 1);
	memcpy(buffer, tag, sz);
	
	sscanf(buffer, "%d.%d.%d", &vnum[0], &vnum[1], &vnum[2]);
	sscanf(gToolName, "" PRNT_CYAN "z64rom " PRNT_GRAY "%d.%d.%d", &cnum[0], &cnum[1], &cnum[2]);
	
	newVer = vnum[0] * 1000 + vnum[1] * 100 + vnum[2];
	curVer = cnum[0] * 1000 + cnum[1] * 100 + cnum[2];
	
	if (newVer > curVer) {
		warn(gLang.setup.update_available, vnum[0], vnum[1], vnum[2]);
		
#ifdef _WIN32
		warn(gLang.setup.update_prompt);
		if (cli_yesno()) {
			sys_exed(x_fmt("tools\\z64upgrade.exe --version \"%s\"", gToolName + strlen("" PRNT_BLUE "z64rom " PRNT_GRAY)));
			
			exit(0);
		}
#endif
		
		info_nl();
	}
	
	Memfile_Free(&api);
	Memfile_Free(&updateChannel);
	return;
	
	error:
	Memfile_Free(&api);
	Memfile_Free(&updateChannel);
	warn(gLang.setup.update_api_limit);
}

/*============================================================================*/

#ifdef _WIN32
const char* ZIP_BINUTIL = "tools/mips64-binutils-win32.zip";
const char* URL_BINUTIL_DOWNLOAD = "https://github.com/z64tools/z64rom/releases/download/binutils/mips64-binutils-win32.zip";
#else
const char* ZIP_BINUTIL = "tools/mips64-binutils-linux.zip";
const char* URL_BINUTIL_DOWNLOAD = "https://github.com/z64tools/z64rom/releases/download/binutils/mips64-binutils-linux.zip";
#endif
const char* ZIP_Z64HDR = "include/z64hdr.zip";
const char* URL_Z64HDR_DOWNLOAD = "https://github.com/z64utils/z64hdr/releases/download/latest/z64hdr.zip";

static int sMaxDownloadAttempt;

static bool Tools_ManualInstall(Memfile* file, const char* name, const char* src_url) {
	const char* filename = NULL;
	
	info(gLang.setup.info_get_file, src_url);
	
	while (!filename) {
		filename = cli_gets();
		
		if (!sys_stat(filename)) {
			putc('\a', stdout);
			cli_clearln(2);
			warn(gLang.setup.filedialog_invalid, filename);
			warn(gLang.setup.closedialog_try_again[2]);
			sys_sleep(2.00f);
			cli_clearln(3);
			filename = NULL;
		}
	}
	
	Memfile_LoadBin(file, filename);
	
	cli_clearln(3);
	
	return EXIT_SUCCESS;
}

static bool Tools_AutoInstall(Memfile* file, const char* name, const char* src_url) {
	int attempt = 0;
	
	while (attempt < sMaxDownloadAttempt) {
		attempt++;
		
		if (!Memfile_Download(file, src_url, gLang.setup.downloading)) {
			cli_clearln(2 - (gInfoProgState));
			return EXIT_SUCCESS;
		}
		
		warn(gLang.setup.warn_download_failed, attempt, sMaxDownloadAttempt);
		sys_sleep(2.0f);
		cli_clearln(2);
	}
	
	return Tools_ManualInstall(file, name, src_url);
}

static bool (*sInstallFunc[])(Memfile*, const char*, const char*) = {
	Tools_ManualInstall,
	Tools_AutoInstall,
};

/*============================================================================*/

static void Tools_InstallImpl(Memfile* file, const char* name, const char* output, const char* src_url, const char* dep) {
	info(name);
	
	sys_touch(dep);
	
	if (!sys_stat(output))
		sInstallFunc[g64.autoInstall](file, name, src_url);
	else
		Memfile_LoadBin(file, output);
	
	sys_rm(dep);
	
	Memfile_SaveBin(file, output);
}

void Tools_InstallBinutils(void) {
	if (sys_stat("tools/mips64-binutils/") && !sys_stat("tools/.gcc_install"))
		return;
	
	Zip zip = {};
	Memfile file = Memfile_New();
	sMaxDownloadAttempt = 4;
	
	Tools_InstallImpl(&file, "mips64-binutils", ZIP_BINUTIL, URL_BINUTIL_DOWNLOAD, "tools/.gcc_install");
	Memfile_Free(&file);
	
	Zip_Load(&zip, ZIP_BINUTIL, 'r');
	Zip_Dump(&zip, "tools/mips64-binutils/", ZipProgressCallback);
	Zip_Free(&zip);
	sys_rm(ZIP_BINUTIL);
	cli_clearln(2);
	
	// mark tools executable on linux following installation
#ifndef _WIN32
	system("find tools/mips64-binutils/ -type f -exec chmod +x {} +");
#endif
	
	info(gLang.success);
}

void Tools_InstallHeader(bool update) {
	if (sys_stat("include/oot_mq_debug/z64hdr.h") && !update && !sys_stat("tools/.z64hdr_install"))
		return;
	
	Zip zip = {};
	Memfile file = Memfile_New();
	g64.autoInstall |= update;
	sMaxDownloadAttempt = 8;
	
	Tools_InstallImpl(&file, "z64hdr", ZIP_Z64HDR, URL_Z64HDR_DOWNLOAD, "tools/.z64hdr_install");
	Memfile_Free(&file);
	
	Zip_Load(&zip, ZIP_Z64HDR, 'r');
	Zip_Dump(&zip, "include/", ZipProgressCallback);
	Zip_Free(&zip);
	sys_rm(ZIP_Z64HDR);
	cli_clearln(2);
	
	sys_mv("include/z64hdr-main/", "include/z64hdr/");
	
	info(gLang.success);
}

/*============================================================================*/

const char* Tools_Get(ToolIndex id) {
	return sTools[id];
}

void Tools_CheckUpdates() {
	if (!sys_stat(updateChannelFilename)) {
		// deprecated auto-update, so don't want this message popping up
		//info("could not find '%s', cannot check for updates", updateChannelFilename);
		return;
	}
	info("warning: auto-update has been deprecated and could behave unpredictably if used,");
	info("         so please refer to the update guide on the z64rom GitHub repository");
	if (!sys_stat("tools/.update-check")) {
		sys_touch("tools/.update-check");
		Tools_CheckUpdateImpl();
	} else {
		date_t now = sys_timedate(sys_time());
		date_t prev = sys_timedate(sys_stat("tools/.update-check"));
		
		if (now.day != prev.day) {
			sys_touch("tools/.update-check");
			Tools_CheckUpdateImpl();
		}
	}
}

s32 Tools_Init(void) {
	if (Tools_ValidateTools_Required())
		return EXIT_PROMPT;
	
	if (Tools_ValidateTools_Additional() || sys_stat("tools/.z64hdr_install") || sys_stat("tools/.gcc_install")) {
		
		if (!g64.autoInstall && !g64.file.gcc64 && !g64.file.z64hdr) {
			if (!g64.chill) {
				info("going to download and install tools...");
				info(gLang.setup.info_chill);
				
				if (cli_yesno())
					Chill();
				
				cli_clearln(2);
				sys_sleep(0.2);
			}
			
			warn(gLang.setup.info_prompt_auto);
			if (cli_yesno())
				g64.autoInstall = true;
			cli_clearln(2);
			sys_sleep(0.2);
		} else if (g64.chill)
			Chill();
		
		Tools_InstallBinutils();
		Tools_InstallHeader(false);
		info(gLang.setup.info_all_ok);
		info_nl();
	}
	
	return EXIT_CONTINUE;
}

/*============================================================================*/
