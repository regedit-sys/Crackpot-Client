#include "stdafx.h"
#include "BO3Menu.h"

#define _bo3menu
#ifdef _bo3menu

unsigned int g_poolSizeBO3[] = { 0x60, 0x5E8, 0x14, 0x58, 0xC4, 0x50, 0x28, 0x10, 0x9C, 0x58, 0x64, 0x0C, 0x190, 0x48, 0x2C, 0x24, 0x468, 0x10, 0x190, 0x00, 0x24, 0x14, 0x08, 0x178, 0x00, 0x00, 0x00, 0x240, 0x384, 0x4C, 0xB0, 0x210, 0xB8, 0x0C, 0x14, 0x0C, 0x18, 0x3C, 0x50, 0x0C, 0x0C, 0x1C, 0x0C, 0x10, 0x48, 0x48C, 0x140, 0xCC, 0x0C, 0x14, 0x20, 0x24, 0x08, 0x40, 0x24, 0x0C, 0x0C, 0x8D8, 0x34, 0x88, 0x10, 0x0C, 0xA4, 0xA4, 0x20, 0x10C, 0x10C, 0x234, 0x1EC, 0xA10, 0x1008, 0x1C, 0x14, 0x34, 0x1E4, 0x58, 0x260, 0x0C, 0x0C, 0x1C, 0x14, 0x10, 0x10, 0x238, 0x90, 0x78, 0x0C, 0x50, 0x1E4, 0x14, 0x0C, 0x18, 0x0C, 0x18, 0x0C, 0x4C, 0x0C, 0x00, 0x3C, 0x24, 0x0C, 0x64, 0x288, 0x14, 0x10, 0x64, 0x44 };

// all the fonts i could find in the xex
char fontnamesBO3[][32] = {
	"fonts/normalFont",
	"fonts/bigFont",
	"fonts/smallFont",
	"fonts/extraBigFont",
	"fonts/extraSmallFont",
	"fonts/boldFont",
	"fonts/consoleFont",
	"fonts/smallDevFont",
	"fonts/bigDevFont"
};


const char *(*vaBO3)(const char *format, ...)
	= (const char *(*)(const char *, ...))0x826D2A68; //TU6

short (*SL_GetStringOfSizeBO3)(const char *text, int user, int length)
	= (short (*)(const char *, int, int))0x822E1720; //TU6

bool (*CG_DObjGetWorldTagPosBO3)(centity_sBO3 *entity, int obj, short tagname, Vector3 &pos)
	= (bool (*)(centity_sBO3 *, int, short, Vector3 &))0x8222A390; //TU6

void (*CG_LocationalTraceBO3)(trace_tBO3 *results, Vector3 &start, Vector3 &end, int passEntityNumber, int contentMask, bool checkRopes, int *context)
	= (void (*)(trace_tBO3 *, Vector3 &, Vector3 &, int, int, bool, int *))0x82294768; //TU6

bool (*BulletPenetrateCheckBO3)(BulletFireParamsBO3 *bp)
	= (bool (*)(BulletFireParamsBO3 *))0x82280C00; //TU6

void (*FireBulletPenetrateBO3)(int localClientNumber, BulletFireParamsBO3 *bp, int weapon, int passEntityNumber, Vector3 &tracerStart, bool drawTracer)
	= (void (*)(int, BulletFireParamsBO3 *, int, int, Vector3 &, bool))0x822818A8; //TU6

GfxCmdDrawText2DBO3 *(*AddBaseDrawTextCmdBO3)(const char *text, int maxChars, Font_sBO3 *font, float x, float y, float xScale, float yScale, Vector4 &color, int style)
	= (GfxCmdDrawText2DBO3 *(*)(const char *, int, Font_sBO3 *, float, float, float, float, Vector4 &, int))0x824C0570; //TU6

void (*SetDrawText2DGlowParmsBO3)(GfxCmdDrawText2DBO3 *cmd, Vector4 &color, Vector4 &glowColor)
	= (void (*)(GfxCmdDrawText2DBO3 *, Vector4 &, Vector4 &))0x824C06C0; //TU6

int (*UI_TextWidthBO3)(const char *text, int maxChars, Font_sBO3 *font, float scale)
	= (int (*)(const char *, int, Font_sBO3 *, float))0x826B4148; //TU6

void (*CG_DrawRotatedPicPhysicalBO3)(int scrPlace, float x, float y, float width, float height, float angle, Vector4 &color, void *material, int)
	= (void (*)(int, float, float, float, float, float, Vector4 &, void *, int))0x8221FB08; //TU6

void (*BG_GetSpreadForWeaponBO3)(playerState_sBO3 *ps, int weapon, float *minSpread, float *maxSpread)
	= (void (*)(playerState_sBO3 *, int, float *, float *))0x821EC308; //TU6


XAssetHeaderBO3 (*DB_FindXAssetHeaderBO3)(XAssetTypeBO3 type, const char *name, bool createIfMissing, int waitTime)
	= (XAssetHeaderBO3 (*)(XAssetTypeBO3, const char *, bool, int))0x82337B28; //TU6

// all the tagnames in the game
char tagnamesBO3[20][32] = {
	"j_helmet",
	"j_head",
	"j_neck",
	"j_shoulder_le",
	"j_shoulder_ri",
	"j_elbow_le",
	"j_elbow_ri",
	"j_wrist_le",
	"j_wrist_ri",
	"j_gun",
	"j_mainroot",
	"j_spineupper",
	"j_spinelower",
	"j_spine4",
	"j_hip_le",
	"j_hip_ri",
	"j_knee_le",
	"j_knee_ri",
	"j_ankle_le",
	"j_ankle_ri"
};

char tagnamesFancyBO3[20][32] = {
	"Helmet",
	"Head",
	"Neck",
	"L Shldr",
	"R Shldr",
	"L Elbow",
	"R Elbow",
	"L Wrist",
	"R Wrist",
	"",
	"Torso",
	"H Spine",
	"L Spine",
	"",
	"L Hip",
	"R Hip",
	"L Knee",
	"R Knee",
	"L Ankle",
	"R Ankle"
};

char aimbotFancyBO3[3][32] = {
	"Normal",
	"Silent",
	"Aimbot Off"
};

char espFancyBO3[3][32] = {
	//"2D Box",
	"3D Box",
	"Pyramid",
	//"Filled Box",
	"ESP Off"
};

int MAIN = 0, AIMBOT = 1, ESP = 2;
int THREE_D = 0, PYRAMID = 1, ESPOFF = 2;
//int TWO_D = 0, THREE_D = 1, PYRAMID = 2, FILLED_BOX = 3, ESPOFF = 4;
int NORMAL = 0, SILENT = 1, AIMOFF = 2;

int currentTagBO3 = 1;
int maxTagBO3 = 20;

int aimbotBO3 = AIMOFF;
int maxAimbot = 3;

int espBO3 = ESPOFF;
int maxESP = 3;

cg_sBO3 *cgBO3;
cgs_tBO3 *cgsBO3;
centity_sBO3 *centsBO3;
clientActive_tBO3 *clientBO3;


bool openBO3 = false;
int mainscrollBO3 = 0;
int espscrollBO3 = 1;
int aimscrollBO3 = 1;
int currentBO3 = MAIN;
int mainmaxBO3 = 6;
char mainmenuBO3[][32] = {
	"Aimbot Options",
	"ESP Options",
	"No Sway",
	"No Recoil",
	"Advanced UAV",
	"Draw Crosshair"
};
bool mainOptionsBO3[6] = { false };
bool *swayBO3 = &mainOptionsBO3[2];
bool *recoilBO3 = &mainOptionsBO3[3];
bool *uavBO3 = &mainOptionsBO3[4];
bool *crosshairBO3 = &mainOptionsBO3[5];

int aimmaxBO3 = 7;
char aimbotmenuBO3[][32] = {
	"Aimbot Type",
	"",
	"Aim Target",
	"",
	"Auto Wall",
	"Auto Fire",
	"No Spread"
};
bool aimOptionsBO3[7] = { false };
bool *autowallBO3 = &aimOptionsBO3[4];
bool *autofireBO3 = &aimOptionsBO3[5];
bool *spreadBO3 = &aimOptionsBO3[6];

int espmaxBO3 = 6;
char espmenuBO3[][32] = {
	"ESP Type",
	"",
	"Show Name",
	"Snap Lines",
	"Show Health",
	"Show Bones"
};
bool espOptionsBO3[6] = { false };
bool *namesBO3 = &espOptionsBO3[2];
bool *linesBO3 = &espOptionsBO3[3];
bool *healthBO3 = &espOptionsBO3[4];
bool *bonesBO3 = &espOptionsBO3[5];

/*char optionsBO3[][32] = {
	"Aimbot",
	"Silent Aim",
	"Auto Wall",
	"Auto Fire",
	"ESP",
	"Bones",
	"Health Bar",
	"Draw Lines",
	"Names",
	"Crosshair",
	"No Spread",
	"No Recoil",
	"UAV",
	"No Sway",
	"",
	"Aim At:",
	""
};*/
const char* fancytagBO3 = "Head";
bool canAutoFireBO3 = false;

// ghetto ass ingame check
bool isInGameBO3() {
	//Dvar_FindDvar = (DWORD (__cdecl*)(char*))0x826B6C38;
	//Dvar_GetBool = (BOOL (__cdecl*)(DWORD))0x826B4A20;
	//DbgPrint("Dvar: %04X", Dvar_FindDvar("cl_ingame"));
	//if (Dvar_GetBool(Dvar_FindDvar("cl_ingame"))) DbgPrint("True");
	return !*(WORD *)0x843FA211 && centsBO3 && cgBO3 && cgsBO3; //TU6
}

short registerTagBO3(const char *tagname) {
	return SL_GetStringOfSizeBO3(tagname, 1, strlen(tagname) + 1);
}

// crazy reversin shit here mate
bool getTagPosBO3(centity_sBO3 *entityBO3, short tagname, Vector3 &pos) {
	int clientObjMap = 0x83E417C8; //TU6
	int r11 = *(short *)(clientObjMap + (entityBO3->clientNumber << 1));
	
	if(!r11)
		return false;

	int objBuf = 0x83DF17A8; //TU6
	int dobj = objBuf + (r11 * 0xA0);
	return CG_DObjGetWorldTagPosBO3(entityBO3, dobj, tagname, pos);
}

short j_headBO3, j_helmetBO3, j_ankle_riBO3, j_ankle_leBO3;
bool getHeadPosBO3(centity_sBO3 *entityBO3, Vector3 &headpos) {
	if(!j_headBO3)
		j_headBO3 = registerTagBO3("j_head");

	return getTagPosBO3(entityBO3, j_headBO3, headpos);
}

bool getNamedTagPosBO3(centity_sBO3 *entityBO3, Vector3 &headpos, const char* tag) {
	short tmp = registerTagBO3(tag);

	return getTagPosBO3(entityBO3, tmp, headpos);
}

float getEntityHeightBO3(centity_sBO3 *entityBO3) {
	if(!j_helmetBO3)
		j_helmetBO3 = registerTagBO3("j_helmet");

	Vector3 helmetpos;
	if(getTagPosBO3(entityBO3, j_helmetBO3, helmetpos)) {
		// add a little so we dont interfere with bone esp
		return (helmetpos.z - entityBO3->origin.z) + 2.0f;
	}

	return 62.0f;
}

// quake 3 source is killa mate
void vectorAnglesBO3(Vector3 &vector, Vector3 &angles) {
	float forward, yaw, pitch;
	
	if(vector.x == 0 && vector.y == 0.0f) {
		yaw = 0.0f;
		if(vector.z > 0.0f) {
			pitch = 90.0f;
		} else {
			pitch = 270.0f;
		}
	} else {
		if(vector.x) {
			yaw = (float)(atan2(vector.y, vector.x) * (180.0f / M_PI)); // degress from radians
		} else if(vector.y > 0.0f) {
			yaw = 90.0f;
		} else {
			yaw = 270.0f;
		}
		
		if(yaw < 0.0f) {
			yaw += 360.0f;
		}

		forward = sqrt((vector.x * vector.x) + (vector.y * vector.y));
		pitch = (float)(atan2(vector.z, forward) * (180.0f / M_PI)); // degress from radians
		
		if(pitch < 0.0f) {
			pitch += 360.0f;
		}
	}

	angles.x = -pitch; // weird ass pitches
	angles.y = yaw;
	angles.z = 0.0f; 
}
void angleVectorsBO3(Vector3 &angles, Vector3 *forward, Vector3 *right, Vector3 *up) {
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles.y * (M_2PI / 360.0f);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * (M_2PI / 360.0f);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * (M_2PI / 360.0f);
	sr = sin(angle);
	cr = cos(angle);

	if(forward) {
		forward->x = cp*cy;
		forward->y = cp*sy;
		forward->z = -sp;
	}

	if(right) {
		right->x = (-1*sr*sp*cy+-1*cr*-sy);
		right->y = (-1*sr*sp*sy+-1*cr*cy);
		right->z = -1*sr*cp;
	}
	
	if(up) {
		up->x = (cr*sp*cy+-sr*-sy);
		up->y = (cr*sp*sy+-sr*cy);
		up->z = cr*cp;
	}
}
void vectorNormalizeBO3(Vector3 vec) {
	float ilength = 1.0f / sqrt(vec.dotProduct(vec));
	vec = vec * ilength;
}

// i could not find any of the functions
// so i made my own :P
bool isEntityVisibleBO3(centity_sBO3 *entityBO3) {
	Vector3 headpos;
	if(!getHeadPosBO3(entityBO3, headpos))
		return false;

	trace_tBO3 trace;
	CG_LocationalTraceBO3(&trace, cgBO3->refdef.viewOrigin, headpos, cgBO3->clientNumber, 0x280F803, false, 0);

	return (trace.fraction >= 0.95f); // fraction of material between two points
}

// hard af
bool canKillEntityBO3(centity_sBO3 *entityBO3) {
	Vector3 headpos;
	if(!getHeadPosBO3(entityBO3, headpos))
		return false;

	BulletFireParamsBO3 bp;
	bp.weaponEntIndex = 0x2BE;
	bp.ignoreEntIndex = cgBO3->localClientNumber;
	bp.damageMultiplier = 1.0f;
	bp.methodOfDeath = 1;
	memcpy(&bp.origStart, &cgBO3->refdef.viewOrigin, 12);
	memcpy(&bp.start, &cgBO3->refdef.viewOrigin, 12);
	memcpy(&bp.end, &headpos, 12);
    Vector3 angles; 
    vectorAnglesBO3(headpos - cgBO3->refdef.viewOrigin, angles);
	angleVectorsBO3(angles, &bp.dir, 0, 0);

	if(!BulletPenetrateCheckBO3(&bp))
		return false;

	// stop shit
	*(short *)0x826B0D58 = 0x4800; // CG_SpawnTracer //TU6
	*(int *)0x822807FC = 0x48000084; // CG_BulletHitEvent //TU6

	FireBulletPenetrateBO3(0, &bp, centsBO3[cgBO3->clientNumber].weapon, cgBO3->clientNumber, cgBO3->refdef.viewOrigin, false);

	// restore shit
	*(short *)0x826B0D58 = 0x4099; // CG_SpawnTracer //TU6
	*(int *)0x822807FC = 0x41820040; // CG_BulletHitEvent //TU6

	//DbgPrint("bp.damageMultiplier: %0.2f\n", bp.damageMultiplier);
	return (bp.ignoreEntIndex == entityBO3->clientNumber);// && bp.damageMultiplier > 0.80f);
}

// just find that shit
int findClosestClientBO3() {
	int closestClient = -1;
	float lastDistance = 1000000.0f;
	for(int i = 0; i < cgsBO3->maxClients; i++) {
		if(i == cgBO3->clientNumber) // client is our own
			continue;

		if(cgBO3->ci[i].team == cgBO3->ci[cgBO3->clientNumber].team) // team check
			if(cgBO3->ci[i].team != 0) // ffa
				continue;

		if(!(centsBO3[i].alive & 2) || cgBO3->ci[i].health < 1) // alive check
			continue;

		/*if(!isEntityVisible(&g->cents[i]) && *visible) // visibility check
			continue; */
		
		if(!canKillEntityBO3(&centsBO3[i]) && *autowallBO3) // auto wall mate
			continue;

		float d = clientBO3->origin.distance(centsBO3[i].origin);
		if(d < lastDistance) {
			lastDistance = d;
			closestClient = i;
		}
	}

	return closestClient;	
}

// reversed from CG_CalcNamePosition
bool worldToScreenBO3(Vector3 &world, Vector2 &screen) {
	// bool CG_CalcNamePosition(int localClientNumber, float *origin, float *x, float *y);

	refdef_sBO3 *refdefBO3 = &cgBO3->refdef;

	Vector3 delta = world - refdefBO3->viewOrigin, projections;
	projections.x = delta.dotProduct(refdefBO3->viewAxis[0]);
	projections.y = delta.dotProduct(refdefBO3->viewAxis[1]);
	projections.z = delta.dotProduct(refdefBO3->viewAxis[2]);

	if(projections.x > 0.0f) {
		// scrPlace->realViewportBase is usually null anyway
		int cx = refdefBO3->width / 2, cy = refdefBO3->height / 2;
		float x = projections.y / refdefBO3->tanHalfFovX;
		screen.x = cx * (1.0f - (x / projections.x));
		float y = projections.z / refdefBO3->tanHalfFovY;
		screen.y = cy * (1.0f - (y / projections.x));

		return true;
	}

	return false;
}

/*
CG_BulletEndpos: 0x8226DE70 tu2
int *randSeed
float f1
float f2
int r6
int r7
int r8
float f3
float f4
float f5,
int arg_54
int arg_5C
int arg_64
int weapon
int shot
int shotCount
*/

// this is all a bitch because it is all inlined on bo3

// ryan your a boss mate
float __declspec(naked) BG_randomBO3(int *randSeed) {
	__asm {
		lis r11, 0x3800
		stw r11, -0x10(r1)
		lfs fp0, -0x10(r1)

		lis r11, 3
		lwz r10, 0(r3)
		ori r8, r11, 0x43FD
		mullw r7, r10, r8
		addis r6, r7, 0x27
		addi r6, r6, -0x613D
		srwi r4, r6, 17
		stw r6, 0(r3)
		std r4, -0x10(r1)
		lfd fp13, -0x10(r1)
		fcfid fp12, fp13
		frsp fp11, fp12
		fmuls fp1, fp11, fp0
		blr
	}
}
void __declspec(naked) generateSeedBO3(int *randSeed) {
	__asm {
		mr r31, r3
		li r10, 3
		mtctr r10
		lwz r3, 0(r31)
		clrlwi r11, r3, 24 // r11 = r3 & 0xFF
	xor:
		slwi r11, r11, 8 // r11 = r11 >> 8
		xor r3, r11, r3 // r3 = r11 ^ r3
		bdnz xor
		stw r3, 0(r31)
		blr
	}
}
void RandomBulletDirBO3(int *randSeed, float *x, float *y) {
	generateSeedBO3(randSeed);

	float theta = (BG_randomBO3(randSeed) * 360.0f) * (M_PI / 180.0f);
	float radius = BG_randomBO3(randSeed);
	*x = radius * cos(theta);
	*y = radius * sin(theta);
}

void compensateSpreadBO3(usercmd_sBO3 *cmdBO3) {
	// only do this if not aiming
	if(!(centsBO3[cgBO3->clientNumber].flags & FLAG_ADSBO3)) {
		float minSpread, maxSpread;
		int weapon = centsBO3[cgBO3->clientNumber].weapon;
		BG_GetSpreadForWeaponBO3(&cgBO3->ps, weapon, &minSpread, &maxSpread);

		float right, up;
		int randSeed = cgBO3->ps.cmdTime;
		RandomBulletDirBO3(&randSeed, &right, &up);

		float aimSpreadScale = cgBO3->aimSpreadScale / 255.0f;
		float aimSpreadAmount = minSpread + ((maxSpread - minSpread) * aimSpreadScale);

		float pitch = up * aimSpreadAmount;
		float yaw = right * aimSpreadAmount;

		cmdBO3->angles[0] += AngleToShort(pitch);
		cmdBO3->angles[1] += AngleToShort(yaw);
	}
}

int __declspec(naked) CL_WritePacketHookStubBO3(int localClientNumber) {
	__asm {
		li r3, 1
		nop
		nop
		nop
		nop
		nop
		nop
		blr
	}
}
int CL_WritePacketHookBO3(int localClientNumber) {
	cgBO3 = (cg_sBO3*)GetPointer(0x82E914B8); //TU6
	cgsBO3 = (cgs_tBO3*)GetPointer(0x82E914A0); //TU6
	centsBO3 = (centity_sBO3*)GetPointer(0x82E914CC); //TU6
	clientBO3 = (clientActive_tBO3*)GetPointer(0x843F5E50); //TU6
	if(isInGameBO3() && aimbotBO3 != AIMOFF) {
		usercmd_sBO3 *cmdBO3 = clientBO3->getUserCmd();
		if((cmdBO3->buttons & BT_LTBO3) || aimbotBO3 == SILENT) {
			int cc = findClosestClientBO3();
			if(cc != -1) {
				Vector3 headpos, angles, newangles;
				getNamedTagPosBO3(&centsBO3[cc], headpos, tagnamesBO3[currentTagBO3]);
				vectorAnglesBO3(headpos - cgBO3->refdef.viewOrigin, angles);
				newangles = angles - clientBO3->spawnAngles;
				if(aimbotBO3 == SILENT) {
					cmdBO3->angles[0] = AngleToShort(newangles.x);
					cmdBO3->angles[1] = AngleToShort(newangles.y);
					//cmdBO3->angles[2] = 0;
					if(*spreadBO3)
						compensateSpreadBO3(cmdBO3);
					//cmd->time += 2;
				} else {
					clientBO3->viewAngles = newangles;
				}
				canAutoFireBO3 = true;
			}
		}
	}

	return CL_WritePacketHookStubBO3(localClientNumber);
}

Font_sBO3 *registerFontBO3(const char *fontname) {
	return DB_FindXAssetHeaderBO3(fontBO3, fontname, 0, -1).font;
}
void *registerMaterialBO3(const char *materialname) {
	return DB_FindXAssetHeaderBO3(materialBO3, materialname, 0, -1).data;
}

Font_sBO3 *smallFontBO3, *smallDevFontBO3;
void *whiteMaterialBO3;
short tagsBO3[20] = { 0 }; // for bone esp

void registerAssetsBO3() {
	if(!smallFontBO3)
		smallFontBO3 = registerFontBO3("fonts/smallFont");

	if(!smallDevFontBO3)
		smallDevFontBO3 = registerFontBO3("fonts/smallDevFont");

	if(!whiteMaterialBO3)
		whiteMaterialBO3 = registerMaterialBO3("white");
}

float getDistanceWorldToScreenBO3(Vector3 &p1, Vector3 &p2) {
	Vector2 xy1, xy2;
	if(worldToScreenBO3(p1, xy1) && worldToScreenBO3(p2, xy2)) {
		return xy1.distance(xy2);
	}
}
float getEntityWidthScreenBO3(centity_sBO3 *entityBO3) {
	Vector3 p1 = entityBO3->origin, p2 = entityBO3->origin;
	p2.z += 35.0f;
	return getDistanceWorldToScreenBO3(p1, p2);
}

void drawStringBO3(const char *text, float x, float y, Font_sBO3 *font, Vector4 &color) {
	AddBaseDrawTextCmdBO3(text, 0x7FFFFFFF, font, x, y, 1.0f, 1.0f, color, 0);
}
void drawStringBO3(const char *text, float x, float y, float xScale, float yScale, Font_sBO3 *font, Vector4 &color) {
	AddBaseDrawTextCmdBO3(text, 0x7FFFFFFF, font, x, y, xScale, yScale, color, 0);
}
void drawMaterialBO3(float x, float y, float width, float height, float angle, Vector4 &color) {
	CG_DrawRotatedPicPhysicalBO3(0x83098998, x, y, width, height, angle, color, whiteMaterialBO3, 0); //TU6
}
void drawLineBO3(float x1, float y1, float x2, float y2, float width, Vector4 &color) {
    float x, y, angle, l1, l2, h1;
    h1 = y2 - y1;
    l1 = x2 - x1;
    l2 = sqrt (l1 * l1 + h1 * h1);
    x = x1 + ((l1 - l2) / 2);
    y = y1 + (h1 / 2);
    angle = (float)atan(h1 / l1) * (180 / 3.14159265358979323846);
	drawMaterialBO3(x, y, l2, width, angle, color);
}
void drawRectangleBO3(float x, float y, float width, float height, float lineWidth, Vector4 &color) {
	drawMaterialBO3(x, y, width, lineWidth, 0.0f, color);
	drawMaterialBO3(x + width, y, lineWidth, height, 0.0f, color);
	drawMaterialBO3(x, y + height - lineWidth, width, lineWidth, 0.0f, color);
	drawMaterialBO3(x, y, lineWidth, height, 0.0f, color);
}
void drawBoxLineBO3(Vector3 &center, float x1, float y1, float z1, float x2, float y2, float z2, Vector4& color) {
	Vector2 xy1, xy2;
	if(worldToScreenBO3(Vector3(center.x + x1, center.y + y1, center.z + z1), xy1) &&
		worldToScreenBO3(Vector3(center.x + x2, center.y + y2, center.z + z2), xy2)) {
		drawLineBO3(xy1.x, xy1.y, xy2.x, xy2.y, 2.5f, color);
	}
}
void draw3DBoxBO3(centity_sBO3 *entity, Vector4 &color) {
	float w = 15.0f, h = getEntityHeightBO3(entity);

	drawBoxLineBO3(entity->origin, -w, -w, 0, w, -w, 0, color);
	drawBoxLineBO3(entity->origin, -w, -w, 0, -w, w, 0, color);
	drawBoxLineBO3(entity->origin, w, w, 0, w, -w, 0, color);
	drawBoxLineBO3(entity->origin, w, w, 0, -w, w, 0, color);

	drawBoxLineBO3(entity->origin, -w, -w, 0, -w, -w, h, color);
	drawBoxLineBO3(entity->origin, -w, w, 0, -w, w, h, color);
	drawBoxLineBO3(entity->origin, w, -w, 0, w, -w, h, color);
	drawBoxLineBO3(entity->origin, w, w, 0, w, w, h, color);

	drawBoxLineBO3(entity->origin, -w, -w, h, w, -w, h, color);
	drawBoxLineBO3(entity->origin, -w, -w, h, -w, w, h, color);
	drawBoxLineBO3(entity->origin, w, w, h, w, -w, h, color);
	drawBoxLineBO3(entity->origin, w, w, h, -w, w, h, color);
}

void drawPyramidBO3(centity_sBO3 *entity, Vector4 &color) {
	float w = 15.0f, h = getEntityHeightBO3(entity)+10;

	drawBoxLineBO3(entity->origin, -w, -w, 0, w, -w, 0, color);
	drawBoxLineBO3(entity->origin, -w, -w, 0, -w, w, 0, color);
	drawBoxLineBO3(entity->origin, w, w, 0, w, -w, 0, color);
	drawBoxLineBO3(entity->origin, w, w, 0, -w, w, 0, color);

	drawBoxLineBO3(entity->origin, -w, -w, 0, 0, 0, h, color);
	drawBoxLineBO3(entity->origin, -w, w, 0, 0, 0, h, color);
	drawBoxLineBO3(entity->origin, w, -w, 0, 0, 0, h, color);
	drawBoxLineBO3(entity->origin, w, w, 0, 0, 0, h, color);
}

void draw2DBoxBO3(centity_sBO3 *entity, Vector4 &color) {
	float w = 15.0f, h = getEntityHeightBO3(entity);

	drawBoxLineBO3(entity->origin, -w, 0, 0, w, 0, 0, color);
	drawBoxLineBO3(entity->origin, -w, 0, 0, -w, 0, h, color);
	drawBoxLineBO3(entity->origin, w, 0, 0, w, 0, h, color);
	drawBoxLineBO3(entity->origin, -w, 0, h, w, 0, h, color);
}

void draw2DFilledBoxBO3(centity_sBO3 *entity, Vector4 &color, Vector4 &color2) {
	float w = 15.0f, h = getEntityHeightBO3(entity);
	Vector2 xy1, xy2;
	if(worldToScreenBO3(Vector3(entity->origin.x - w, entity->origin.y + 0, entity->origin.z + 0), xy1) &&
		worldToScreenBO3(Vector3(entity->origin.x + w, entity->origin.y + 0, entity->origin.z + h), xy2)) {
		drawLineBO3(xy1.x, xy1.y, xy2.x, xy2.y, 2.5f, color2);
		drawBoxLineBO3(entity->origin, -w, 0, 0, w, 0, 0, color);
		drawBoxLineBO3(entity->origin, -w, 0, 0, -w, 0, h, color);
		drawBoxLineBO3(entity->origin, w, 0, 0, w, 0, h, color);
		drawBoxLineBO3(entity->origin, -w, 0, h, w, 0, h, color);
	}
}

	
void drawHealthBO3(centity_sBO3 *entity) {
	Vector2 screen;
	if(worldToScreenBO3(entity->origin, screen)){
		float w = getEntityWidthScreenBO3(entity), health = ((float)cgBO3->ci[entity->clientNumber].health / 100.0f);
		float cw = w / 2.0f, wh = w / 8.0f;
		drawMaterialBO3(screen.x - cw, screen.y, w, wh, 0, red);
		drawMaterialBO3(screen.x - cw, screen.y, w * health, wh, 0, green);
		drawRectangleBO3((screen.x - cw), screen.y, w + 1.0f, wh + 1.0f, 0.5f, black);
	}
}
void drawBoneLineBO3(centity_sBO3 *entity, short tagname1, short tagname2, Vector4& color) {
	Vector3 world1, world2;
	if(getTagPosBO3(entity, tagname1, world1) && getTagPosBO3(entity, tagname2, world2)) {
		Vector2 xy1, xy2;
		if(worldToScreenBO3(world1, xy1) && worldToScreenBO3(world2, xy2)) {
			float w = 1.0f; // + cgBO3->ps.weaponPosFraction; // for ads
			drawLineBO3(xy1.x, xy1.y, xy2.x, xy2.y, w, color);
		}
	}
}

void drawBonesBO3(centity_sBO3 *entity) {
	if(!tagsBO3[0]) {
		for(int i = 0; i < 20; i++) 
			tagsBO3[i] = registerTagBO3(tagnamesBO3[i]);
	}

	Vector3 tagpos1, tagpos2;

	// for quick replacement
	Vector4 color = red;
	

	// top: j_helmet to j_neck
	drawBoneLineBO3(entity, tagsBO3[0], tagsBO3[1], color);
	drawBoneLineBO3(entity, tagsBO3[1], tagsBO3[2], color);

	// left arm: j_neck to j_wrist_le
	drawBoneLineBO3(entity, tagsBO3[2], tagsBO3[3], color);
	drawBoneLineBO3(entity, tagsBO3[3], tagsBO3[5], color);
	drawBoneLineBO3(entity, tagsBO3[5], tagsBO3[7], color);

	// right arm: j_neck to j_wrist_ri
	drawBoneLineBO3(entity, tagsBO3[2], tagsBO3[4], color);
	drawBoneLineBO3(entity, tagsBO3[4], tagsBO3[6], color);
	drawBoneLineBO3(entity, tagsBO3[6], tagsBO3[8], color);

	// center: j_neck to j_spinelower
	drawBoneLineBO3(entity, tagsBO3[2], tagsBO3[10], color);
	drawBoneLineBO3(entity, tagsBO3[10], tagsBO3[11], color);
	drawBoneLineBO3(entity, tagsBO3[11], tagsBO3[12], color);
	drawBoneLineBO3(entity, tagsBO3[12], tagsBO3[13], color);

	

	// left leg: j_spine4 to j_ankle_le
	drawBoneLineBO3(entity, tagsBO3[13], tagsBO3[14], color);
	drawBoneLineBO3(entity, tagsBO3[14], tagsBO3[16], color);
	drawBoneLineBO3(entity, tagsBO3[16], tagsBO3[18], color);

	// right leg: j_spine4 to j_ankle_ri
	drawBoneLineBO3(entity, tagsBO3[13], tagsBO3[15], color);
	drawBoneLineBO3(entity, tagsBO3[15], tagsBO3[17], color);
	drawBoneLineBO3(entity, tagsBO3[17], tagsBO3[19], color);
}

XINPUT_STATE XInputStateBO3;
short currentButtonBO3, previousButtonBO3;
#define buttonHitBO3(button) ((currentButtonBO3 & ~previousButtonBO3) & button)
void updateInputBO3() {
	XInputGetState(0, &XInputStateBO3);
	previousButtonBO3 = currentButtonBO3;
	currentButtonBO3 = ~XInputStateBO3.Gamepad.wButtons & 0xFBFF;

	if(buttonHitBO3(XINPUT_GAMEPAD_DPAD_LEFT)) {
		openBO3 = !openBO3;
	}

	if(openBO3) {
		if (currentBO3 == AIMBOT || currentBO3 == ESP){
			if(buttonHitBO3(XINPUT_GAMEPAD_B)) {
				currentBO3 = MAIN;
			}
		}
		if(buttonHitBO3(XINPUT_GAMEPAD_DPAD_UP)) {
			if (currentBO3 == MAIN){
				mainscrollBO3--;
				if(mainscrollBO3 == -1) {
					mainscrollBO3 = mainmaxBO3 - 1;
				}
				DbgPrint("Main Scroll Value: %d", mainscrollBO3);
			} else if (currentBO3 == AIMBOT){
				aimscrollBO3--;
				if(aimscrollBO3 == 0 || aimscrollBO3 == -1){
					aimscrollBO3 = aimmaxBO3 - 1;
				} else if (aimscrollBO3 == 2){
					aimscrollBO3 = 1;
				}
				DbgPrint("Aim Scroll Value: %d", aimscrollBO3);
			} else if (currentBO3 == ESP){
				espscrollBO3--;
				if(espscrollBO3 == 0 || espscrollBO3 == -1){
					espscrollBO3 = espmaxBO3 - 1;
				}
				DbgPrint("ESP Scroll Value: %d", espscrollBO3);
			}
		}

		if(buttonHitBO3(XINPUT_GAMEPAD_DPAD_DOWN)) {
			if (currentBO3 == MAIN){
				mainscrollBO3++;
				if(mainscrollBO3 == mainmaxBO3) {
					mainscrollBO3 = 0;
				}
				DbgPrint("Main Scroll Value: %d", mainscrollBO3);
			} else if (currentBO3 == AIMBOT){
				aimscrollBO3++;
				if(aimscrollBO3 == 0 || aimscrollBO3 == aimmaxBO3){
					aimscrollBO3 = 1;
				} else if (aimscrollBO3 == 2){
					aimscrollBO3 = 3;
				}
				DbgPrint("Aim Scroll Value: %d", aimscrollBO3);
			} else if (currentBO3 == ESP){
				espscrollBO3++;
				if(espscrollBO3 == 0 || espscrollBO3 == espmaxBO3){
					espscrollBO3 = 1;
				}
				DbgPrint("ESP Scroll Value: %d", espscrollBO3);
			}
		}

		if(buttonHitBO3(XINPUT_GAMEPAD_X)) {
			if (currentBO3 == MAIN){
				if (mainscrollBO3 == 0){
					currentBO3 = AIMBOT;
				} else if (mainscrollBO3 == 1){
					currentBO3 = ESP;
				} else {
					mainOptionsBO3[mainscrollBO3] = !mainOptionsBO3[mainscrollBO3];
				}
			} else if (currentBO3 == AIMBOT){
				if (aimscrollBO3 == 1){
					switch(aimbotBO3){
						case 2:
							aimbotBO3 = NORMAL;
							break;
						case 0:
							aimbotBO3 = SILENT;
							break;
						case 1:
							aimbotBO3 = AIMOFF;
							break;
						default:
							aimbotBO3 = AIMOFF;
							break;
					}
					DbgPrint("Aimbot Value: %d", aimbotBO3);
				} else if (aimscrollBO3 == 3){
					currentTagBO3++;
					if (currentTagBO3 == 9 || currentTagBO3 == 13){
						currentTagBO3++;
					}
					if (currentTagBO3 >= maxTagBO3){
						currentTagBO3 = 0;
					}
					//DbgPrint("Tag Number: %d", currentTag);
					fancytagBO3 = tagnamesFancyBO3[currentTagBO3];
				} else {
					if(aimscrollBO3 == 6) return;
					aimOptionsBO3[aimscrollBO3] = !aimOptionsBO3[aimscrollBO3];
				}
			} else if (currentBO3 == ESP){
				if (espscrollBO3 == 1){
					switch(espBO3){
						case 2:
							espBO3 = THREE_D;
							break;
						case 0:
							espBO3 = PYRAMID;
							break;
						case 1:
							espBO3 = ESPOFF;
							break;
						default:
							espBO3 = ESPOFF;
							break;
					}
					DbgPrint("ESP Value: %d", espBO3);
				} else {
					espOptionsBO3[espscrollBO3] = !espOptionsBO3[espscrollBO3];
				}
			}
		}
	}
}
void updateOptionsBO3() {
	// redboxes for tu1
	// *(int *)0x825FC2C0 = 0x60000000;
	// *(int *)0x825FC2E8 = 0x60000000;

	// BG_GetSpreadForWeapon
	// do my own shit for silent aim
	if(*spreadBO3 && aimbotBO3 != SILENT)
		*(int *)0x821EC320 = 0x39400002; // li r10, 2 //TU6
	else
		*(int *)0x821EC320 = 0x89432E6D; // lbz r10, 0x2E6D(r3) //TU6

	// BG_WeaponFireRecoil
	if(*recoilBO3)
		*(int *)0x82288E18 = 0x60000000; // nop //TU6
	else
		*(int *)0x82288E18 = 0x4BF79A89; // bl BG_WeaponFireRecoil //TU6

	if(*uavBO3)
		*(int *)0x8229AB40 = 0x60000000; // nop //TU6
	else
		*(int *)0x8229AB40 = 0x419A004C; // beq //TU6

	// BG_SetWeaponMovementAngles
	// BG_CalculateViewMovement_BobAngles
	if(*swayBO3) {
		*(int *)0x82210970 = 0x60000000; // nop //TU6
		*(int *)0x8221112C = 0x60000000; // nop //TU6
	} else {
		*(int *)0x82210970 = 0x4BFFE659; // bl BG_SetWeaponMovementAngles //TU6
		*(int *)0x8221112C = 0x4BFFFBA5; // bl BG_CalculateViewMovement_BobAngles //TU6
	}

	if(*crosshairBO3){
		*(int *)0x8209A208 = 0x3f800000;
	} else {
		*(int *)0x8209A208 = 0x43700000;
	}
}

void CG_DrawDebugHookBO3() {
	cgBO3 = (cg_sBO3*)GetPointer(0x82E914B8); //TU6
	cgsBO3 = (cgs_tBO3*)GetPointer(0x82E914A0); //TU6
	centsBO3 = (centity_sBO3*)GetPointer(0x82E914CC); //TU6
	clientBO3 = (clientActive_tBO3*)GetPointer(0x843F5E50); //TU6


	registerAssetsBO3();

	drawStringBO3("^1Syndicate", 20.0f, 30.0f, smallFontBO3, white); // hack name

	if(isInGameBO3()) {
		float x = cgBO3->refdef.width, y = cgBO3->refdef.height;
		float cx = x / 2.0f, cy = y / 2.0f;

		// do menu shit
		updateInputBO3();
		updateOptionsBO3();
		if(openBO3) {
			if (currentBO3 == MAIN){
				drawMaterialBO3(x - 155.0f, 0.0f, 155.0f, 20.0f + (mainmaxBO3 * 20.0f), 0, Vector4(0.0f, 0.0f, 0.0f, 0.75f));
				drawRectangleBO3(x - 159.0f, 0.0f, 155.0f, 20.0f + (mainmaxBO3 * 20.0f), 4.0f, red);

				for(int i = 0; i < mainmaxBO3; i++) {
					if(i == mainscrollBO3) {
						drawMaterialBO3(x - 145.0f, 10.0f + (i * 20.0f), 130.0f, 20.0f, 0, Vector4(1.0f, 1.0f, 1.0f, 0.50f));
					}
					if (i == 0 || i == 1){
						float w = UI_TextWidthBO3(mainmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(mainmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, purple);
					} else {
						float w = UI_TextWidthBO3(mainmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(mainmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, mainOptionsBO3[i] ? red : Gray);
					}
				}
			} else if (currentBO3 == AIMBOT){
				drawMaterialBO3(x - 155.0f, 0.0f, 155.0f, 20.0f + (aimmaxBO3 * 20.0f), 0, Vector4(0.0f, 0.0f, 0.0f, 0.75f));
				drawRectangleBO3(x - 159.0f, 0.0f, 155.0f, 20.0f + (aimmaxBO3 * 20.0f), 4.0f, red);

				for(int i = 0; i < aimmaxBO3; i++) {
					if(i == aimscrollBO3) {
						drawMaterialBO3(x - 145.0f, 10.0f + (i * 20.0f), 130.0f, 20.0f, 0, Vector4(1.0f, 1.0f, 1.0f, 0.50f));
					}
					if (i == 0 || i == 2){
						float w = UI_TextWidthBO3(aimbotmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(aimbotmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, purple);
					} else if (i == 1){
						float w = UI_TextWidthBO3(aimbotFancyBO3[aimbotBO3], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(aimbotFancyBO3[aimbotBO3], x - 120.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, cyan);
					} else if (i == 3){
						float w = UI_TextWidthBO3(tagnamesFancyBO3[currentTagBO3], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(tagnamesFancyBO3[currentTagBO3], x - 120.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, cyan);
					} else {
						float w = UI_TextWidthBO3(aimbotmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(aimbotmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, aimOptionsBO3[i] ? red : Gray);
					}
				}
			} else if (currentBO3 == ESP){
				drawMaterialBO3(x - 155.0f, 0.0f, 155.0f, 20.0f + (espmaxBO3 * 20.0f), 0, Vector4(0.0f, 0.0f, 0.0f, 0.75f));
				drawRectangleBO3(x - 159.0f, 0.0f, 155.0f, 20.0f + (espmaxBO3 * 20.0f), 4.0f, red);

				for(int i = 0; i < espmaxBO3; i++) {
					if(i == espscrollBO3) {
						drawMaterialBO3(x - 145.0f, 10.0f + (i * 20.0f), 130.0f, 20.0f, 0, Vector4(1.0f, 1.0f, 1.0f, 0.50f));
					}
					if (i == 0){
						float w = UI_TextWidthBO3(espmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(espmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, purple);
					} else if (i == 1){
						float w = UI_TextWidthBO3(espFancyBO3[espBO3], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(espFancyBO3[espBO3], x - 120.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, cyan);
					} else {
						float w = UI_TextWidthBO3(espmenuBO3[i], 0x7FFFFFF, smallFontBO3, 1.0f);
						drawStringBO3(espmenuBO3[i], x - 140.0f, 30.0f + (20.0f * i), 0.8f, 0.7f, smallFontBO3, espOptionsBO3[i] ? red : Gray);
					}
				}
			} else {
				drawMaterialBO3(x - 155.0f, 0.0f, 155.0f, 35.0f, 0, Vector4(0.0f, 0.0f, 0.0f, 0.75f));
				drawRectangleBO3(x - 159.0f, 0.0f, 155.0f, 35.0f, 4.0f, red);
				drawStringBO3("Syndicate", x - 130.0f, 30.0f, smallFontBO3, red);
			}
		} else {
			drawMaterialBO3(x - 155.0f, 0.0f, 155.0f, 35.0f, 0, Vector4(0.0f, 0.0f, 0.0f, 0.75f));
			drawRectangleBO3(x - 159.0f, 0.0f, 155.0f, 35.0f, 4.0f, red);
			drawStringBO3("Syndicate", x - 130.0f, 30.0f, smallFontBO3, red);
		}
		drawStringBO3("Press ^BXENONButtondpadL^ To Open Menu | Press ^BXENONButtonX^ to Select", 400.0f, 700.0f, smallFontBO3, red);
		
		for(int i = 0; i < cgsBO3->maxClients; i++) {
			Vector2 screen, screenh;
			centity_sBO3 *entity = &centsBO3[i];
			if(worldToScreenBO3(entity->origin, screen)) {
				if(i == cgBO3->clientNumber) // client is our own
					continue;

				if(!(entity->alive & 2) || cgBO3->ci[i].health < 1) // alive check
					continue;

				if(cgBO3->ci[i].team == cgBO3->ci[cgBO3->clientNumber].team) // team check
					if(cgBO3->ci[i].team != 0) // ffa
						continue;

				Vector4 color = canKillEntityBO3(entity) ? red : gray1;
				Vector4 color2 = canKillEntityBO3(entity) ? red : Gray;

				// not in order of menu bc we want stuff to draw over other certain things

				if(*bonesBO3)
					drawBonesBO3(entity); // draw bones

				//if(espBO3 == TWO_D)
				//	draw2DBoxBO3(entity, color);
				//else 
				if (espBO3 == THREE_D)
					draw3DBoxBO3(entity, color);
				else if (espBO3 == PYRAMID)
					drawPyramidBO3(entity, color);
				//else if (espBO3 == FILLED_BOX)
				//	draw2DFilledBoxBO3(entity, color, trans_red);

				if(*healthBO3)
					drawHealthBO3(entity); // health bar

				if(*linesBO3)
					drawLineBO3(cx, cy, screen.x, screen.y, 3.0f, color); // snap line
				
				if(*namesBO3) {
					Vector3 headpos;
					getHeadPosBO3(entity, headpos);
					if(worldToScreenBO3(headpos, screenh)) {
						float d = cgBO3->origin.distance(entity->origin) * 0.0254f;
						const char *text = vaBO3("%s [%0.2fm]", cgBO3->ci[i].name, d);
						float w = UI_TextWidthBO3(text, 0x7FFFFFFF, smallDevFontBO3, 0.75f);
						drawStringBO3(text, screenh.x - (w / 4.0f), screenh.y, 0.75f, 0.75f, smallDevFontBO3, color2); // name
					}
				}
			}

			// custom crosshair
			// do this here so it will overlay esp lines names etc
			if(*crosshairBO3) {
				drawLineBO3(cx, cy - 15.0f, cx, cy + 15.0f, 2.0f, red); // vertical
				drawLineBO3(cx - 15.0f, cy, cx + 15.0f, cy, 2.0f, red); // horizontal

				// outline vertical
				drawLineBO3(cx - 2.0f, cy - 14.0f, cx - 2.0f, cy + 14.0f, 2.0f, black);
				drawLineBO3(cx + 2.0f, cy - 14.0f, cx + 2.0f, cy + 14.0f, 2.0f, black);

				// outline horizontal
				drawLineBO3(cx - 14.0f, cy - 2.0f, cx + 14.0f, cy - 2.0f, 2.0f, black);
				drawLineBO3(cx - 14.0f, cy + 2.0f, cx + 14.0f, cy + 2.0f, 2.0f, black);
			}

		}
	} else {
		openBO3 = false;
	}
}

DWORD XamInputGetStateHookBO3(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_STATE pState) {
	DWORD r = XInputGetStateEx(dwUserIndex, dwFlags, pState);

	if(*autofireBO3 && canAutoFireBO3) {
		pState->Gamepad.bRightTrigger = 100;
		canAutoFireBO3 = false;
	}

	return r;
}

int __declspec(naked) UI_SafeTranslateStringHookStubBO3(const char *reference) {
	__asm {
		li r3, 2
		nop
		nop
		nop
		nop
		nop
		nop
		blr
	}
}

// custom suicide messages!
char suicideMessagesBO3[][64] = {
	"&&1 took one for the team m8",
	"&&1 got fucked up.",
	"&&1 took dick in the ass",
	"&&1 just got hit with a RKO!!!",
	"&&1 was fucking slayed!",
};

const char *UI_SafeTranslateStringHookBO3(const char *reference) {
	if(!strcmp(reference, "CGAME_QUAKE_SUICIDE")) {
		return suicideMessagesBO3[rand() % (sizeof(suicideMessagesBO3) / 64)];
	}

	// have to cast bc it errors if i put a const char * in the naked stub
	return (const char *)UI_SafeTranslateStringHookStubBO3(reference);
}


VOID DoBO3Menu() {
	DbgPrint("BO3 Menu Loaded");
	openBO3 = false;
	mainscrollBO3 = 0;
	espscrollBO3 = 1;
	aimscrollBO3 = 1;
	currentBO3 = 0;
	espBO3 = 2;
	aimbotBO3 = 2;
	currentTagBO3 = 1;
	//HookFunctionStart((PDWORD)0x82304CE8, (PDWORD)CL_WritePacketHookStubBO3, (DWORD)CL_WritePacketHookBO3); //TU6
	//PatchInJump((PDWORD)0x82220990, (DWORD)CG_DrawDebugHookBO3, FALSE); //TU6
	PLDR_DATA_TABLE_ENTRY tableEntry;
	XexPcToFileHeader((PVOID)0x82000000, &tableEntry);
	//PatchModuleImport(tableEntry, "xam.xex", 0x191, (DWORD)XamInputGetStateHookBO3);
	//HookFunctionStart((PDWORD)0x826B0200, (PDWORD)UI_SafeTranslateStringHookStubBO3, (DWORD)UI_SafeTranslateStringHookBO3); //TU6
	cgBO3 = (cg_sBO3*)GetPointer(0x82E914B8); //TU6
	cgsBO3 = (cgs_tBO3*)GetPointer(0x82E914A0); //TU6
	centsBO3 = (centity_sBO3*)GetPointer(0x82E914CC); //TU6
	clientBO3 = (clientActive_tBO3*)GetPointer(0x843F5E50); //TU6
	XNotifyQueueUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, 0, 2, L"Syndicate - Black Ops 3\nMenu + Aimbot Enabled!", 0);
}
#endif