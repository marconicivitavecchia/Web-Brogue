/*
 *  Globals.c
 *  UnBrogue
 *
 *  Brogue created by Brian Walker on 1/10/09.
 *  Copyright 2012. All rights reserved.
 *  
 *  UnBrogue created by Andrew Doull on 1/8/12.
 *  Copyright 2012-2013. All rights reserved.
 *
 *  This file is part of UnBrogue, a 'variant' of Brogue in the
 *  tradition of Angband variants.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"

tcell tmap[DCOLS][DROWS];						// grids with info about the map
pcell pmap[DCOLS][DROWS];
short **scentMap;
cellDisplayBuffer displayBuffer[COLS][ROWS];	// used to optimize plotCharWithColor
short terrainRandomValues[DCOLS][DROWS][8];
char buffer[DCOLS][DROWS];						// used in cave generation
short **safetyMap;								// used to help monsters flee
short **allySafetyMap;							// used to help allies flee
short **chokeMap;								// used to assess the importance of the map's various chokepoints
short **playerPathingMap;						// used to calculate routes for mouse movement
short listOfWallsX[4][DROWS*DCOLS];
short listOfWallsY[4][DROWS*DCOLS];
short numberOfWalls[4];
const short nbDirs[8][2] = {{0,-1},{0,1},{-1,0},{1,0},{-1,-1},{-1,1},{1,-1},{1,1}};
const short cDirs[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
const short tDirs[3][3] = {{UPLEFT, LEFT, DOWNLEFT}, {UP, NO_DIRECTION, DOWN}, {UPRIGHT, RIGHT, DOWNRIGHT}}; // used to translate (newX - oldX +1, newY-oldY+1) to dir
const uchar impaleDirs[8] = {IMPALE_UP, IMPALE_DOWN, IMPALE_LEFT, IMPALE_RIGHT, IMPALE_UPLEFT, IMPALE_DOWNLEFT, IMPALE_UPRIGHT,IMPALE_DOWNRIGHT}; 
short numberOfRooms;
short numberOfWaypoints;
levelData levels[101];
creature player;
playerCharacter rogue;
creature *monsters;
creature *dormantMonsters;
creature *graveyard;
item *floorItems;
item *packItems;
item *monsterItemsHopper;
room *rooms;
waypoint waypoints[MAX_WAYPOINTS];
levelProfile *thisLevelProfile;

char displayedMessage[MESSAGE_LINES][COLS*2];
boolean messageConfirmed[MESSAGE_LINES];
char combatText[COLS * 2];
short messageArchivePosition;
char messageArchive[MESSAGE_ARCHIVE_LINES][COLS*2];

char currentFilePath[BROGUE_FILENAME_MAX];

char displayDetail[DCOLS][DROWS];		// used to make certain per-cell data accessible to external code (e.g. terminal adaptations)

#ifdef AUDIT_RNG
FILE *RNGLogFile;
#endif

unsigned char inputRecordBuffer[INPUT_RECORD_BUFFER + 10];
unsigned short locationInRecordingBuffer;
unsigned long randomNumbersGenerated;
unsigned long positionInPlaybackFile;
unsigned long lengthOfPlaybackFile;
unsigned long recordingLocation;
unsigned long maxLevelChanges;
char annotationPathname[BROGUE_FILENAME_MAX];	// pathname of annotation file
unsigned long previousGameSeed;

#pragma mark Colors
//									Red		Green	Blue	RedRand	GreenRand	BlueRand	Rand	Dances?
// basic colors
const color white =					{100,	100,	100,	0,		0,			0,			0,		false};
const color gray =					{50,	50,		50,		0,		0,			0,			0,		false};
const color darkGray =				{30,	30,		30,		0,		0,			0,			0,		false};
const color veryDarkGray =			{15,	15,		15,		0,		0,			0,			0,		false};
const color black =					{0,		0,		0,		0,		0,			0,			0,		false};
const color yellow =				{100,	100,	0,		0,		0,			0,			0,		false};
const color darkYellow =			{50,	50,		0,		0,		0,			0,			0,		false};
const color teal =					{30,	100,	100,	0,		0,			0,			0,		false};
const color purple =				{100,	0,		100,	0,		0,			0,			0,		false};
const color darkPurple =			{50,	0,		50,		0,		0,			0,			0,		false};
const color brown =					{60,	40,		0,		0,		0,			0,			0,		false};
const color green =					{0,		100,	0,		0,		0,			0,			0,		false};
const color darkGreen =				{0,		50,		0,		0,		0,			0,			0,		false};
const color orange =				{100,	50,		0,		0,		0,			0,			0,		false};
const color darkOrange =			{50,	25,		0,		0,		0,			0,			0,		false};
const color blue =					{0,		0,		100,	0,		0,			0,			0,		false};
const color darkBlue =				{0,		0,		50,		0,		0,			0,			0,		false};
const color lightBlue =				{40,	40,		100,	0,		0,			0,			0,		false};
const color pink =					{100,	60,		66,		0,		0,			0,			0,		false};
const color red  =					{100,	0,		0,		0,		0,			0,			0,		false};
const color darkRed =				{50,	0,		0,		0,		0,			0,			0,		false};
const color tanColor =				{80,	67,		15,		0,		0,			0,			0,		false};

// bolt colors
const color rainbow =				{-70,	-70,	-70,	170,	170,		170,		0,		true};
// const color rainbow =			{0,		0,		50,		100,	100,		0,			0,		true};
const color descentBoltColor =      {-40,   -40,    -40,    0,      0,          80,         80,     true};
const color discordColor =			{25,	0,		25,		66,		0,			0,			0,		true};
const color poisonColor =			{0,		0,		0,		10,		50,			10,			0,		true};
const color beckonColor =			{10,	10,		10,		5,		5,			5,			50,		true};
const color invulnerabilityColor =	{25,	0,		25,		0,		0,			66,			0,		true};
const color dominationColor =		{0,		0,		100,	80,		25,			0,			0,		true};
const color fireBoltColor =			{500,	150,	0,		45,		30,			0,			0,		true};
const color flamedancerCoronaColor ={500,	150,	100,	45,		30,			0,			0,		true};
//const color shieldingColor =		{100,	50,		0,		0,		50,			100,			0,		true};
const color shieldingColor =		{150,	75,		0,		0,		50,			175,		0,		true};

// tile colors
const color undiscoveredColor =		{0,		0,		0,		0,		0,			0,			0,		false};

const color wallForeColor =			{7,		7,		7,		3,		3,			3,			0,		false};

color wallBackColor;
const color wallBackColorStart =	{45,	40,		40,		15,		0,			5,			20,		false};
const color wallBackColorEnd =		{40,	30,		35,		0,		20,			30,			20,		false};

const color graniteBackColor =		{10,	10,		10,		0,		0,			0,			0,		false};

const color floorForeColor =		{30,	30,		30,		0,		0,			0,			35,		false};

color floorBackColor;
const color floorBackColorStart =	{2,		2,		10,		2,		2,			0,			0,		false};
const color floorBackColorEnd =		{5,		5,		5,		2,		2,			0,			0,		false};

const color stairsBackColor =		{15,	15,		5,		0,		0,			0,			0,		false};
const color firstStairsBackColor =	{10,	10,		25,		0,		0,			0,			0,		false};

const color refuseBackColor =		{6,		5,		3,		2,		2,			0,			0,		false};
const color rubbleBackColor =		{7,		7,		8,		2,		2,			1,			0,		false};
const color bloodflowerForeColor =  {30,    5,      40,     5,      1,          3,          0,      false};
const color bloodflowerPodForeColor = {50,  5,      25,     5,      1,          3,          0,      false};
const color bloodflowerBackColor =  {15,    3,      10,     3,      1,          3,          0,      false};

const color obsidianBackColor =		{6,		0,		8,		2,		0,			3,			0,		false};
const color carpetForeColor =		{23,	30,		38,		0,		0,			0,			0,		false};
const color carpetBackColor =		{15,	8,		5,		0,		0,			0,			0,		false};
const color doorForeColor =			{70,	35,		15,		0,		0,			0,			0,		false};
const color doorBackColor =			{30,	10,		5,		0,		0,			0,			0,		false};
//const color ironDoorForeColor =		{40,	40,		40,		0,		0,			0,			0,		false};
const color ironDoorForeColor =		{500,	500,	500,	0,		0,			0,			0,		false};
const color ironDoorBackColor =		{15,	15,		30,		0,		0,			0,			0,		false};
const color vaultDoorForeColor =	{500,	500,	500,	0,		0,			0,			0,		false};
const color vaultDoorBackColor =	{50,	40,		0,		0,		0,			0,			0,		false};
const color bridgeFrontColor =		{33,	12,		12,		12,		7,			2,			0,		false};
const color bridgeBackColor =		{12,	3,		2,		3,		2,			1,			0,		false};
const color statueBackColor =		{20,	20,		20,		0,		0,			0,			0,		false};
const color glyphColor =            {20,    5,      5,      50,     0,          0,          0,      true};
const color glyphLightColor =       {150,   0,      0,      150,    0,          0,          0,      true};

//const color deepWaterForeColor =	{5,		5,		40,		0,		0,			10,			10,		true};
//color deepWaterBackColor;
//const color deepWaterBackColorStart = {5,	5,		55,		5,		5,			10,			10,		true};
//const color deepWaterBackColorEnd =	{5,		5,		45,		2,		2,			5,			5,		true};
//const color shallowWaterForeColor =	{40,	40,		90,		0,		0,			10,			10,		true};
//color shallowWaterBackColor;
//const color shallowWaterBackColorStart ={30,30,		80,		0,		0,			10,			10,		true};
//const color shallowWaterBackColorEnd ={20,	20,		60,		0,		0,			5,			5,		true};

const color deepWaterForeColor =	{5,		8,		20,		0,		4,			15,			10,		true};
color deepWaterBackColor;
const color deepWaterBackColorStart = {5,	10,		31,		5,		5,			5,			6,		true};
const color deepWaterBackColorEnd =	{5,		8,		20,		2,		3,			5,			5,		true};
const color shallowWaterForeColor =	{28,	28,		60,		0,		0,			10,			10,		true};
color shallowWaterBackColor;
const color shallowWaterBackColorStart ={20,20,		60,		0,		0,			10,			10,		true};
const color shallowWaterBackColorEnd ={12,	15,		40,		0,		0,			5,			5,		true};

const color mudForeColor =			{18,	14,		5,		5,		5,			0,			0,		false};
const color mudBackColor =			{23,	17,		7,		5,		5,			0,			0,		false};
const color chasmForeColor =		{7,		7,		15,		4,		4,			8,			0,		false};
color chasmEdgeBackColor;
const color chasmEdgeBackColorStart ={5,	5,		25,		2,		2,			2,			0,		false};
const color chasmEdgeBackColorEnd =	{8,		8,		20,		2,		2,			2,			0,		false};
const color fireForeColor =			{70,	20,		0,		15,		10,			0,			0,		true};
const color lavaForeColor =			{20,	20,		20,		100,	10,			0,			0,		true};
const color brimstoneForeColor =	{100,	50,		10,		0,		50,			40,			0,		true};
const color brimstoneBackColor =	{18,	12,		9,		0,		0,			5,			0,		false};

const color lavaBackColor =			{70,	20,		0,		15,		10,			0,			0,		true};
const color acidBackColor =			{15,	80,		25,		5,		15,			10,			0,		true};

const color lightningColor =		{100,	150,	500,	50,		50,			0,			50,		true};
const color fungusLightColor =		{2,		11,		11,		4,		3,			3,			0,		true};
const color lavaLightColor =		{47,	13,		0,		10,		7,			0,			0,		true};
const color deepWaterLightColor =	{10,	30,		100,	0,		30,			100,		0,		true};

const color grassColor =			{15,	40,		15,		15,		50,			15,			10,		false};
const color deadGrassColor =		{20,	13,		0,		20,		10,			5,			10,		false};
const color fungusColor =			{15,	50,		50,		0,		25,			0,			30,		true};
const color grayFungusColor =		{30,	30,		30,		5,		5,			5,			10,		false};
const color foliageColor =			{25,	100,	25,		15,		0,			15,			0,		false};
const color deadFoliageColor =		{20,	13,		0,		30,		15,			0,			20,		false};
const color lichenColor =			{50,	5,		25,		10,		0,			5,			0,		true};
const color lichenFungusColor =		{50,	5,		25,		10,		25,			5,			25,		true};
const color hayColor =				{70,	55,		5,		0,		20,			20,			0,		false};
const color ashForeColor =			{20,	20,		20,		0,		0,			0,			20,		false};
const color bonesForeColor =		{80,	80,		30,		5,		5,			35,			5,		false};
const color ectoplasmColor =		{45,	20,		55,		25,		0,			25,			5,		false};
const color forceFieldColor =		{0,		25,		25,		0,		25,			25,			0,		true};
const color wallCrystalColor =		{40,	40,		60,		20,		20,			40,			0,		true};
const color altarForeColor =		{5,		7,		9,		0,		0,			0,			0,		false};
const color altarBackColor =		{35,	18,		18,		0,		0,			0,			0,		false};
const color pedestalBackColor =		{10,	5,		20,		0,		0,			0,			0,		false};
const color graffitiForeColor =		{30,	20,		0,		0,		0,			0,			0,		false};
const color graffitiBackColor =		{0,		0,		0,		0,		0,			0,			0,		false};
const color mandrakeRootColor =		{100,	45,		50,		10,		30,			30,			0,		true};

// monster colors
const color goblinColor =			{40,	30,		20,		0,		0,			0,			0,		false};
const color jackalColor =			{60,	42,		27,		0,		0,			0,			0,		false};
const color ogreColor =				{60,	25,		25,		0,		0,			0,			0,		false};
const color eelColor =				{30,	12,		12,		0,		0,			0,			0,		false};
const color goblinConjurerColor =	{67,	10,		100,	0,		0,			0,			0,		false};
const color spectralBladeColor =	{15,	15,		60,		0,		0,			70,			50,		true};
const color spectralImageColor =	{13,	0,		0,		25,		0,			0,			0,		true};
const color toadColor =				{40,	65,		30,		0,		0,			0,			0,		false};
const color trollColor =			{40,	60,		15,		0,		0,			0,			0,		false};
const color centipedeColor =		{75,	25,		85,		0,		0,			0,			0,		false};
const color dragonColor =			{20,	80,		15,		0,		0,			0,			0,		false};
const color krakenColor =			{100,	55,		55,		0,		0,			0,			0,		false};
const color salamanderColor =		{40,	10,		0,		8,		5,			0,			0,		true};
const color pixieColor =			{60,	60,		60,		40,		40,			40,			0,		true};
const color darPriestessColor =		{0,		50,		50,		0,		0,			0,			0,		false};
const color darMageColor =			{50,	50,		0,		0,		0,			0,			0,		false};
const color wraithColor =			{66,	66,		25,		0,		0,			0,			0,		false};
const color pinkJellyColor =		{100,	40,		40,		5,		5,			5,			20,		true};
const color wormColor =				{80,	60,		40,		0,		0,			0,			0,		false};
const color sentinelColor =			{3,		3,		30,		0,		0,			10,			0,		true};
const color goblinMysticColor =		{10,	67,		100,	0,		0,			0,			0,		false};
const color ifritColor =			{50,	10,		100,	75,		0,			20,			0,		true};
const color phoenixColor =			{100,	0,		0,		0,		100,		0,			0,		true};

// light colors
color minersLightColor;
const color minersLightStartColor =	{180,	180,	180,	0,		0,			0,			0,		false};
const color minersLightEndColor =	{90,	90,		120,	0,		0,			0,			0,		false};
const color torchColor =			{150,	75,		30,		0,		30,			20,			0,		true};
const color torchLightColor =		{75,	38,		15,		0,		15,			7,			0,		true};
//const color hauntedTorchColor =     {75,	30,		150,	30,		20,			0,			0,		true};
const color hauntedTorchColor =     {75,	20,		40,     30,		10,			0,			0,		true};
//const color hauntedTorchLightColor ={19,     7,		37,		8,		4,			0,			0,		true};
const color hauntedTorchLightColor ={67,    10,		10,		20,		4,			0,			0,		true};
const color ifritLightColor =		{0,		10,		150,	100,	0,			100,		0,		true};
//const color unicornLightColor =		{-50,	-50,	-50,	200,	200,		200,		0,		true};
const color unicornLightColor =		{-50,	-50,	-50,	250,	250,		250,		0,		true};
const color wispLightColor =		{75,	100,	250,	33,		10,			0,			0,		true};
const color summonedImageLightColor ={200,	0,		75,		0,		0,			0,			0,		true};
const color spectralBladeLightColor ={40,	0,		230,	0,		0,			0,			0,		true};
const color ectoplasmLightColor =	{23,	10,		28,		13,		0,			13,			3,		false};
const color explosionColor =		{10,	8,		2,		0,		2,			2,			0,		true};
const color dartFlashColor =		{500,	500,	500,	0,		2,			2,			0,		true};
const color lichLightColor =		{-50,	80,		30,		0,		0,			20,			0,		true};
const color forceFieldLightColor =	{10,	10,		10,		0,		50,			50,			0,		true};
const color crystalWallLightColor =	{10,	10,		10,		0,		0,			50,			0,		true};
const color sunLightColor =			{100,	100,	75,		0,		0,			0,			0,		false};
const color sunLightCloudColor =	{110,	110,	80,		0,		20,			20,			10,		true};
const color fungusForestLightColor ={30,	40,		60,		0,		0,			0,			40,		true};
const color fungusTrampledLightColor ={10,	10,		10,		0,		50,			50,			0,		true};
const color redFlashColor =			{100,	10,		10,		0,		0,			0,			0,		false};
const color darknessPatchColor =	{-10,	-10,	-10,	0,		0,			0,			0,		false};
const color darknessCloudColor =	{-20,	-20,	-20,	0,		0,			0,			0,		false};
const color magicMapFlashColor =	{60,	20,		60,		0,		0,			0,			0,		false};
const color sentinelLightColor =	{20,	20,		120,	10,		10,			60,			0,		true};
const color telepathyColor =		{30,	30,		130,	0,		0,			0,			0,		false};
const color markedColor =			{130,	20,		50,		0,		0,			0,			0,		false};
const color confusionLightColor =	{10,	10,		10,		10,		10,			10,			0,		true};
const color portalActivateLightColor ={300,	400,	500,	0,		0,			0,			0,		true};
const color descentLightColor =     {20,    20,     70,     0,      0,          0,          0,      false};
const color algaeBlueLightColor =   {20,    15,     50,     0,      0,          0,          0,      false};
const color algaeGreenLightColor =  {15,    50,     20,     0,      0,          0,          0,      false};

// color multipliers
const color colorDim25 =			{25,	25,		25,		25,		25,			25,			25,		false};
const color colorMultiplier100 =	{100,	100,	100,	100,	100,		100,		100,	false};
const color memoryColor =			{25,	25,		50,		20,		20,			20,			0,		false};
const color memoryOverlay =			{25,	25,		50,		0,		0,			0,			0,		false};
const color magicMapColor =			{60,	20,		60,		60,		20,			60,			0,		false};
const color clairvoyanceColor =		{50,	90,		50,		50,		90,			50,			66,		false};
//const color telepathyMultiplier =	{40,	40,		110,	40,		40,			110,		66,		false};
const color telepathyMultiplier =	{30,	30,		130,	30,		30,			130,		66,		false};
const color markedMultiplier =		{130,	20,		50,		130,	20,			50,			66,		false};
const color omniscienceColor =		{140,	100,	60,		140,	100,		60,			90,		false};
const color basicLightColor =		{180,	180,	180,	180,	180,		180,		180,	false};

// blood colors
const color humanBloodColor =		{60,	20,		10,		15,		0,			0,			15,		false};
const color insectBloodColor =		{10,	60,		20,		0,		15,			0,			15,		false};
const color vomitColor =			{60,	50,		5,		0,		15,			15,			0,		false};
const color urineColor =			{70,	70,		40,		0,		0,			0,			10,		false};
const color methaneColor =			{45,	60,		15,		0,		0,			0,			0,		false};

// gas colors
const color poisonGasColor =		{75,	25,		85,		0,		0,			0,			0,		false};
const color confusionGasColor =		{60,	60,		60,		40,		40,			40,			0,		true};
const color windColor =				{20,	20,		30,		0,		0,			0,			40,		false};

// interface colors
const color itemColor =				{100,	95,		-30,	0,		0,			0,			0,		false};
const color blueBar =				{15,	10,		50,		0,		0,			0,			0,		false};
const color redBar =				{45,	10,		15,		0,		0,			0,			0,		false};
const color hiliteColor =			{100,	100,	0,		0,		0,			0,			0,		false};
const color interfaceBoxColor =		{7,		6,		15,		0,		0,			0,			0,		false};
const color interfaceButtonColor =	{18,	15,		38,		0,		0,			0,			0,		false};
const color buttonHoverColor =		{100,	70,		40,		0,		0,			0,			0,		false};
const color titleButtonColor =		{23,	15,		30,		0,		0,			0,			0,		false};

const color playerInvisibleColor =  {20,    20,     30,     0,      0,          80,         0,      true};
const color playerInLightColor =	{100,	100,	30,		0,		0,			0,			0,		false};
const color playerInShadowColor =	{60,	60,		100,	0,		0,			0,			0,		false};
const color playerInDarknessColor =	{30,	30,		65,		0,		0,			0,			0,		false};

const color goodMessageColor =		{60,	50,		100,	0,		0,			0,			0,		false};
const color badMessageColor =		{100,	50,		60,		0,		0,			0,			0,		false};
const color advancementMessageColor ={50,	100,	60,		0,		0,			0,			0,		false};
const color itemMessageColor =		{100,	100,	50,		0,		0,			0,			0,		false};
const color flavorTextColor =		{50,	40,		90,		0,		0,			0,			0,		false};
const color backgroundMessageColor ={60,	20,		70,		0,		0,			0,			0,		false};

//const color flameSourceColor = {0, 0, 0, 65, 40, 100, 0, true}; // 1
const color flameSourceColor = {0, 0, 0, 80, 50, 100, 0, true}; // 2
//const color flameSourceColor = {25, 13, 25, 50, 25, 50, 0, true}; // 3
//const color flameSourceColor = {20, 20, 20, 60, 20, 40, 0, true}; // 4
//const color flameSourceColor = {20, 0, -20, 60, 60, 120, 0, true}; // 5**
//const color flameSourceColor = {10, 0, -10, 40, 30, 60, 50, true}; // 6
//const color flameSourceColor = {30, 18, 18, 70, 36, 36, 0, true}; // 7**
//const color flameSourceColor = {20, 7, 7, 60, 40, 40, 0, true}; // 8

const color flameTitleColor = {0, 0, 0, 17, 10, 6, 0, true}; // pale orange
//const color flameTitleColor = {0, 0, 0, 7, 7, 10, 0, true}; // *pale blue*
//const color flameTitleColor = {0, 0, 0, 9, 9, 15, 0, true}; // *pale blue**
//const color flameTitleColor = {0, 0, 0, 11, 11, 18, 0, true}; // *pale blue*
//const color flameTitleColor = {0, 0, 0, 15, 15, 9, 0, true}; // pale yellow
//const color flameTitleColor = {0, 0, 0, 15, 9, 15, 0, true}; // pale purple

#pragma mark Dynamic color references

const color *dynamicColors[NUMBER_DYNAMIC_COLORS][3] = {
	// used color			shallow color				deep color
	{&minersLightColor,		&minersLightStartColor,		&minersLightEndColor},
	{&wallBackColor,		&wallBackColorStart,		&wallBackColorEnd},
	{&deepWaterBackColor,	&deepWaterBackColorStart,	&deepWaterBackColorEnd},
	{&shallowWaterBackColor,&shallowWaterBackColorStart,&shallowWaterBackColorEnd},
	{&floorBackColor,		&floorBackColorStart,		&floorBackColorEnd},
	{&chasmEdgeBackColor,	&chasmEdgeBackColorStart,	&chasmEdgeBackColorEnd},
};

#pragma mark Autogenerator definitions

const autoGenerator autoGeneratorCatalog[NUMBER_AUTOGENERATORS] = {
//	 terrain					layer	DF							Machine						reqDungeon  reqLiquid   >Depth	<Depth	freq	minIncp	minSlope	maxNumber
	{0,							0,		DF_GRANITE_COLUMN,			0,							FLOOR,		NOTHING,    1,		100,	60,		100,	0,			4},
	{0,							0,		DF_CRYSTAL_WALL,			0,							TOP_WALL,	NOTHING,    14,		100,	15,		-325,	25,			5},
	{0,							0,		DF_LUMINESCENT_FUNGUS,		0,							FLOOR,		NOTHING,    7,		100,	15,		-300,	70,			14},
	{0,							0,		DF_GRASS,					0,							FLOOR,		NOTHING,    0,		10,		0,		1000,	-80,		10},
	{0,							0,		DF_DEAD_GRASS,				0,							FLOOR,		NOTHING,    4,		9,		0,		-200,	80,			10},
	{0,							0,		DF_DEAD_GRASS,				0,							FLOOR,		NOTHING,    9,		14,		0,		1200,	-80,		10},
	{0,							0,		DF_BONES,					0,							FLOOR,		NOTHING,    12,		99,     30,		0,		0,			4},
	{0,							0,		DF_RUBBLE,					0,							FLOOR,		NOTHING,    0,		99,     30,		0,		0,			4},
	{0,							0,		DF_FOLIAGE,					0,							FLOOR,		NOTHING,    0,		8,		15,		1000,	-333,		10},
	{0,							0,		DF_FUNGUS_FOREST,			0,							FLOOR,		NOTHING,    13,		100,	30,		-600,	50,			12},
	{0,							0,		DF_BUILD_ALGAE_WELL,		0,							FLOOR,      DEEP_WATER, 10,		100,	50,		0,      0,			2},
	{STATUE_INERT,				DUNGEON,0,							0,							TOP_WALL,	NOTHING,    6,		99,     5,		-100,	35,			3},
	{STATUE_INERT,				DUNGEON,0,							0,							FLOOR,		NOTHING,    10,		99,     50,		0,		0,			3},
	{TORCH_WALL,				DUNGEON,0,							0,							TOP_WALL,	NOTHING,    6,		99,     5,		-200,	70,			12},
	{GAS_TRAP_POISON_HIDDEN,	DUNGEON,0,							0,							FLOOR,		NOTHING,    5,		99,     30,		100,	0,			3},
	{0,                         0,      0,							MT_PARALYSIS_TRAP_AREA,		FLOOR,		NOTHING,    7,		99,     30,		100,	0,			3},
	{TRAP_DOOR_HIDDEN,			DUNGEON,0,							0,							FLOOR,		NOTHING,    9,		99,     30,		100,	0,			2},
	{GAS_TRAP_CONFUSION_HIDDEN,	DUNGEON,0,							0,							FLOOR,		NOTHING,    11,		99,     30,		100,	0,			3},
	{FLAMETHROWER_HIDDEN,		DUNGEON,0,							0,							FLOOR,		NOTHING,    13,		99,     30,		100,	0,			3},
	{FLOOD_TRAP_HIDDEN,			DUNGEON,0,							0,							FLOOR,		NOTHING,    15,		99,     30,		100,	0,			3},
	{0,							0,		0,							MT_SWAMP_AREA,				FLOOR,		NOTHING,    1,		99,     30,		0,		0,			2},
	{0,							0,		DF_SUNLIGHT,				0,							FLOOR,		NOTHING,    0,		5,		15,		500,	-150,		10},
	{0,							0,		DF_DARKNESS,				0,							FLOOR,		NOTHING,    1,		15,		15,		500,	-50,		10},
	{STEAM_VENT,				DUNGEON,0,							0,							FLOOR,		NOTHING,    16,		99,     20,		100,	0,			3},
	{WIND_VENT,					DUNGEON,0,							0,							FLOOR,		NOTHING,	8,		99,		10,		100,	0,			3},
	{CRYSTAL_WALL,              DUNGEON,0,                          0,                          TOP_WALL,   NOTHING,    100,    100,    100,    0,      0,          1000},
	{CRYSTAL_WALL,              DUNGEON,0,                          0,                          PERM_WALL,  NOTHING,    100,    100,    100,    0,      0,          1000},
	{0,							0,		DF_LUMINESCENT_FUNGUS,		0,							FLOOR,		NOTHING,    100,	100,	100,	0,      0,			1000},
	{0,							0,		0,							MT_STARTER_STAFF_OR_MELEE,	FLOOR,		NOTHING,	1,		1,		5,		0,		0,			1},
	{0,							0,		0,							MT_STARTER_TALISMAN_LIBRARY,FLOOR,		NOTHING,	1,		1,		1,		0,		0,			1},
	{0,							0,		0,							MT_CRYPT_AREA,				FLOOR,		NOTHING,	4,		19,		5,		0,		0,			1},
	{0,							0,		0,							MT_POTION_EXPERIMENT_AREA,	FLOOR,		NOTHING,	10,		19,		5,		0,		0,			2},
	{0,							0,		0,							MT_SCROLL_EXPERIMENT_AREA,	FLOOR,		NOTHING,	10,		19,		5,		0,		0,			2},
	{0,							0,		0,							MT_SACRIFICE_AREA,			FLOOR,		NOTHING,	1,		19,		5,		0,		0,			1},
	{0,							0,		0,							MT_CAPACITOR_AREA,			FLOOR,		NOTHING,	11,		99,		5,		0,		0,			1},
	{0,                         0,      0,                          MT_WITCH_HAZEL_AREA,        FLOOR,      NOTHING,    5,      30,     10,		0,		0,			3},
	{0,                         0,      0,                          MT_MANDRAKE_ROOT_AREA,      FLOOR,      NOTHING,    7,      30,     5,		0,		0,			2},
	{0,                         0,      0,                          MT_STINKFRUIT_AREA,			FLOOR,		NOTHING,	4,      30,     5,		0,		0,			2},
	{0,                         0,      0,                          MT_CRIMSON_CAP_AREA,		FLOOR,		NOTHING,	4,      30,     2,		0,		0,			2},
	{0,                         0,      0,                          MT_BLOODFLOWER_AREA,        FLOOR,      NOTHING,    1,      30,     15,     140,    -10,        3},
	{0,							0,		0,							MT_IDYLL_AREA,				FLOOR,		NOTHING,    1,		5,		15,		0,		0,			1},
	{0,							0,		0,							MT_REMNANT_AREA,			FLOOR,		NOTHING,    10,		100,	15,		0,		0,			2},
	{0,							0,		0,							MT_DISMAL_AREA,				FLOOR,		NOTHING,    7,		100,	12,		0,		0,			5},
	{0,							0,		0,							MT_BRIDGE_TURRET_AREA,		FLOOR,		NOTHING,    5,		99,     6,		0,		0,			2},
	{0,							0,		0,							MT_LAKE_PATH_TURRET_AREA,	FLOOR,		NOTHING,    5,		99,     6,		0,		0,			2},
	{0,							0,		0,							MT_TRICK_STATUE_AREA,		FLOOR,		NOTHING,    6,		99,     15,		0,		0,			3},
	{0,							0,		0,							MT_SENTINEL_AREA,			FLOOR,		NOTHING,    12,		20,		10,		0,		0,			2},
	{0,							0,		0,							MT_WORM_AREA,				FLOOR,		NOTHING,    12,		20,		12,		0,		0,			3},
};

#pragma mark Terrain definitions

const floorTileType tileCatalog[NUMBER_TILETYPES] = {
	
	// promoteChance is in hundredths of a percent per turn
	
	//	char		fore color				back color		priority	ignit	fireType	discovType	promoteType		promoteChance	glowLight		flags																								description			flavorText
	
	// dungeon layer (this layer must have all of fore color, back color and char)
	{	' ',		&black,					&black,					100,0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"a chilly void",		""},
	{WALL_CHAR,		&wallBackColor,			&graniteBackColor,		0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a rough granite wall",	"The granite is split open with splinters of rock jutting out at odd angles."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"the ground",			""},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"the ground",			""},
	{FLOOR_CHAR,	&carpetForeColor,		&carpetBackColor,		85,	0,	DF_EMBERS,		0,			0,				0,				NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE),														"the carpet",			"Ornate carpeting fills this room, a relic of ages past."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a stone wall",			"The rough stone wall is firm and unyielding."},
	{DOOR_CHAR,		&doorForeColor,			&doorBackColor,			25,	50,	DF_EMBERS,		0,			DF_OPEN_DOOR,	0,				NO_LIGHT,		(T_OBSTRUCTS_VISION | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_PROMOTES_ON_STEP | T_STAND_IN_TILE), "a wooden door",	"you pass through the doorway."},
	{OPEN_DOOR_CHAR,&doorForeColor,			&doorBackColor,			25,	50,	DF_EMBERS,		0,			DF_CLOSED_DOOR,	10000,			NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),										"an open door",			"you pass through the doorway."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	50,	DF_EMBERS,		DF_SHOW_DOOR,0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_IS_SECRET | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),	"a stone wall",		"The rough stone wall is firm and unyielding."},
	{DOOR_CHAR,		&ironDoorForeColor,		&ironDoorBackColor,		15,	50,	DF_EMBERS,		0,			DF_OPEN_IRON_DOOR_INERT,0,		NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_PROMOTES_WITH_KEY | T_STAND_IN_TILE),		"a locked iron door",	"you search your pack but do not have a matching key."},
	{OPEN_DOOR_CHAR,&white,					&ironDoorBackColor,		90,	50,	DF_EMBERS,		0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE | T_OBSTRUCTS_SURFACE_EFFECTS),													"an open iron door",	"you pass through the doorway."},
	{OPEN_DOOR_CHAR,&white,					&ironDoorBackColor,		90,	50,	DF_EMBERS,		0,			DF_OPEN_IRON_DOOR_ON,0,			NO_LIGHT,		(T_STAND_IN_TILE | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),			"an open iron door",	"the doorway shifts slightly as you pass through it.", (T2_PROMOTES_WITH_ENTRY)},
	{OPEN_DOOR_CHAR,&white,					&ironDoorBackColor,		90,	50,	DF_EMBERS,		0,			DF_OPEN_IRON_DOOR_OFF,0,		NO_LIGHT,		(T_STAND_IN_TILE | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),			"an open iron door",	"the doorway shifts slightly as you pass through it.", (T2_PROMOTES_WITHOUT_ENTRY)},
	{DOOR_CHAR,		&ironDoorForeColor,		&vaultDoorBackColor,	15,	50,	DF_EMBERS,		0,			DF_OPEN_IRON_DOOR_INERT,0,		NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_PROMOTES_WITH_KEY | T_STAND_IN_TILE),		"a locked vault door",	"you search your pack but do not have a matching key."},
	{OPEN_DOOR_CHAR,&white,					&vaultDoorBackColor,	90,	50,	DF_EMBERS,		0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE | T_OBSTRUCTS_SURFACE_EFFECTS),													"an open vault door",	"you pass through the doorway."},
	{WALL_CHAR,		&wallForeColor,			&obsidianBackColor,		10,	0,	0,				0,			0,				0,				LAVA_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"an obsidian barrier",	"this obsidian barrier seems to absorb the light. You can't see through it."},
	{DESCEND_CHAR,	&itemColor,				&stairsBackColor,		30,	0,	DF_PLAIN_FIRE,	0,			DF_REPEL_CREATURES, 0,			NO_LIGHT,		(T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS | T_PROMOTES_ON_STEP),								"a downward staircase",	"stairs spiral downward into the depths."},
	{ASCEND_CHAR,	&itemColor,				&stairsBackColor,		30,	0,	DF_PLAIN_FIRE,	0,			DF_REPEL_CREATURES, 0,			NO_LIGHT,		(T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS | T_PROMOTES_ON_STEP),								"an upward staircase",	"stairs spiral upward."},
	{OMEGA_CHAR,	&lightBlue,				&firstStairsBackColor,	30,	0,	DF_PLAIN_FIRE,	0,			DF_REPEL_CREATURES, 0,			NO_LIGHT,		(T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS | T_PROMOTES_ON_STEP),								"the dungeon exit",		"the gilded doors leading out of the dungeon are sealed by an invisible force."},
	{WALL_CHAR,		&torchColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				TORCH_LIGHT,	(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a wall-mounted torch",	"The torch is anchored firmly to the wall and sputters quietly in the gloom."},
	{WALL_CHAR,		&wallCrystalColor,		&wallCrystalColor,		0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				CRYSTAL_WALL_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),"a crystal formation", "You feel the crystal's glossy surface and admire the dancing lights beneath.", (T2_REFLECTS_BOLTS)},
	{WALL_CHAR,		&gray,					&floorBackColor,		10,	0,	DF_PLAIN_FIRE,	0,			DF_OPEN_PORTCULLIS,	0,			NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED), "a heavy portcullis",	"The iron bars rattle but will not budge; they are firmly locked in place."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			DF_ACTIVATE_PORTCULLIS,0,		NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_IS_WIRED),															"the ground",			""},
	{WALL_CHAR,		&doorForeColor,			&floorBackColor,		10,	100,DF_WOODEN_BARRICADE_BURN,0,	DF_ADD_WOODEN_BARRICADE,10000,	NO_LIGHT,		(T_OBSTRUCTS_ITEMS | T_STAND_IN_TILE | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),					"a dry wooden barricade", "The wooden barricade is firmly set but has dried over the years. Might it burn?"},
	{WALL_CHAR,		&doorForeColor,			&floorBackColor,		10,	100,DF_WOODEN_BARRICADE_BURN,0,	0,				0,				NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_STAND_IN_TILE | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),"a dry wooden barricade","The wooden barricade is firmly set but has dried over the years. Might it burn?"},
	{WALL_CHAR,		&torchLightColor,		&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_PILOT_LIGHT,	0,				TORCH_LIGHT,	(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),				"a wall-mounted torch",	"The torch is anchored firmly to the wall, and sputters quietly in the gloom."},
	{FIRE_CHAR,		&fireForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				TORCH_LIGHT,	(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_FIRE),												"a fallen torch",		"The torch lies at the foot of the wall, spouting gouts of flame haphazardly."},
    {WALL_CHAR,		&torchColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_HAUNTED_TORCH_TRANSITION,0,	TORCH_LIGHT,	(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),				"a wall-mounted torch",	"The torch is anchored firmly to the wall and sputters quietly in the gloom."},
    {WALL_CHAR,		&torchColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_HAUNTED_TORCH,2000,			TORCH_LIGHT,	(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),                             "a wall-mounted torch",	"The torch is anchored firmly to the wall and sputters quietly in the gloom."},
    {WALL_CHAR,		&hauntedTorchColor,		&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				HAUNTED_TORCH_LIGHT,(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),                                                     "a sputtering torch",	"A dim purple flame sputters and spits atop this wall-mounted torch."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	DF_REVEAL_LEVER,0,			0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_SECRET),											"a stone wall",			"The rough stone wall is firm and unyielding."},
    {LEVER_CHAR,	&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_PULL_LEVER,  0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_PROMOTES_ON_PLAYER_ENTRY | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),"a lever", "You pull the lever embedded in the wall."},
    {LEVER_PULLED_CHAR,&wallForeColor,		&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),                                                         "an inactive lever",    "The lever won't budge."},
    {WALL_CHAR,		&wallForeColor,         &wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,          DF_CREATE_LEVER,0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_WIRED),											"a stone wall",			"The rough stone wall is firm and unyielding."},
    {STATUE_CHAR,	&wallBackColor,			&statueBackColor,		0,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),			"a marble statue",		"The cold marble statue has weathered the years with grace."},
	{STATUE_CHAR,	&wallBackColor,			&statueBackColor,		0,	0,	DF_PLAIN_FIRE,	0,			DF_CRACKING_STATUE,0,			NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),"a marble statue",	"The cold marble statue has weathered the years with grace."},
	{STATUE_CHAR,	&wallBackColor,			&statueBackColor,		0,	0,	DF_PLAIN_FIRE,	0,			DF_STATUE_SHATTER,3500,			NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"a cracking statue",	"Deep cracks ramble down the side of the statue even as you watch."},
	{OMEGA_CHAR,	&wallBackColor,			&floorBackColor,		17,	0,	DF_PLAIN_FIRE,	0,			DF_PORTAL_ACTIVATE,0,			NO_LIGHT,		(T_OBSTRUCTS_ITEMS | T_IS_WIRED | T_STAND_IN_TILE),													"a stone archway",		"This ancient moss-covered stone archway radiates a strange, alien energy."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_TURRET_EMERGE,0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),				"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_WALL_SHATTER,0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),				"a stone wall",			"The rough stone wall is firm and unyielding."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			DF_DARKENING_FLOOR,	0,			NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION),															"the ground",			""},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			DF_DARK_FLOOR,	1500,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION),																		"the ground",			""},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			0,				0,				DARKNESS_CLOUD_LIGHT,0,																								"the ground",			""},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_PROMOTES_ON_PLAYER_ENTRY | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),								"the ground",			""},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			0,				0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS),																		"a candle-lit altar",	"a gilded altar is adorned with candles that flicker in the breeze."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_CAGE_APPEARS,0,				CANDLE_LIGHT,	(T_PROMOTES_WITHOUT_KEY | T_VANISHES_UPON_PROMOTION),												"a candle-lit altar",	"a gilded altar is adorned with candles that flicker in the breeze."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			0,				0,				CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),							"an iron cage",			"the cage won't budge."},
	{GEM_CHAR,		&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			0,				0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_PROMOTES_WITH_KEY),									"a candle-lit altar",	"ornate gilding spirals around a spherical depression in the top of the altar."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_ITEM_CAGE_CLOSE,	0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_PROMOTES_WITHOUT_KEY),	"a candle-lit altar",	"a cage, open on the bottom, hangs over this altar on a retractable chain."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_ITEM_CAGE_OPEN,	0,			CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_PROMOTES_WITH_KEY | T_STAND_IN_TILE),"an iron cage","the missing item must be replaced before you can access the remaining items."},
	{ALTAR_CHAR,	&altarBackColor,		&veryDarkGray,			17,	0,	DF_EMBERS,		0,			DF_ALTAR_SWITCH_ON,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a candle-lit altar",	"the altar clicks as you stand on it.", (T2_PROMOTES_WITH_ENTRY)},
	{ALTAR_CHAR,	&altarBackColor,		&veryDarkGray,			17,	0,	DF_EMBERS,		0,			DF_ALTAR_SWITCH_OFF,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a candle-lit altar",	"the altar clicks as you stand on it.", (T2_PROMOTES_WITHOUT_ENTRY)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_ALTAR_SWITCH_CLOSE,0,		CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),								"a candle-lit altar",	"a cage, open on the bottom, hangs over this altar on a retractable chain."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_ALTAR_SWITCH_OPEN,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","there is something missing from here, stolen from the dungeon long ago."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_DOOR_CAGE_CLOSE,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),								"a candle-lit altar",	"a cage, open on the bottom, hangs over this altar on a retractable chain."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_DOOR_CAGE_OPEN,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","there is something missing from here, stolen from the dungeon long ago."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_ALTAR_DETECT_KEY,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_PROMOTES_ON_PLAYER_ENTRY | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),"a candle-lit altar",	"there is something missing from here, stolen by a thief!"}, // wiring only needed to ensure altar grid gets machine ID
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_ALTAR_INERT,	0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a candle-lit altar",	"a weathered stone altar is adorned with candles that flicker in the breeze.", (T2_PROMOTES_ON_ITEM_PICKUP)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_ALTAR_RETRACT,0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a candle-lit altar",	"a weathered stone altar is adorned with candles that flicker in the breeze.", (T2_PROMOTES_ON_ITEM_PICKUP)},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_CAGE_DISAPPEARS,	0,			CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","the cage won't budge. Perhaps there is a way to raise it nearby..."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_GODS_REVEAL,	0,				CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","the cage won't budge. There's no way to raise it, but by the gods."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_GODS_REWARD,	0,				CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","the cage won't budge. There's no way to raise it, but by the gods."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_GODS_PUNISH,	0,				CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"an iron cage","the cage won't budge. There's no way to raise it, but by the gods."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_GODS_GAMBLED,0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_IS_WIRED),															"a candle-lit altar",	"a weathered stone altar. There is space to place an item here.", (T2_PROMOTES_WITH_GAMBLE)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			0,				0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS),																		"a candle-lit altar",	"a weathered stone altar. May the gods grant you a favour next time."},
	{GRAFFITI_CHAR,	&graffitiForeColor,		&graffitiBackColor,		17,	0,	0,				0,			DF_GRAFFITI_1,	0,				NO_LIGHT,		(T_PROMOTES_ON_PLAYER_ENTRY),																		"scrawled graffiti",	"someone has scrawled a message here. You need to be close to read it."},
	{GRAFFITI_CHAR,	&graffitiForeColor,		&graffitiBackColor,		17,	0,	0,				0,			DF_GRAFFITI_2,	0,				NO_LIGHT,		(T_PROMOTES_ON_PLAYER_ENTRY),																		"scrawled graffiti",	"someone has scrawled a message here. You need to be close to read it."},
	{GRAFFITI_CHAR,	&graffitiForeColor,		&graffitiBackColor,		17,	0,	0,				0,			DF_GRAFFITI_3,	0,				NO_LIGHT,		(T_PROMOTES_ON_PLAYER_ENTRY),																		"scrawled graffiti",	"someone has scrawled a message here. You need to be close to read it."},
	{GRAFFITI_CHAR,	&graffitiForeColor,		&graffitiBackColor,		17,	0,	0,				0,			DF_GRAFFITI_4,	0,				NO_LIGHT,		(T_PROMOTES_ON_PLAYER_ENTRY),																		"scrawled graffiti",	"someone has scrawled a message here. You need to be close to read it."},
	{ALTAR_CHAR,	&altarForeColor,		&pedestalBackColor,		17, 0,	0,				0,			0,				0,				CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS),																		"a stone pedestal",		"elaborate carvings wind around this ancient pedestal."},
	{ALTAR_CHAR,	&floorBackColor,		&veryDarkGray,			17, 0,	0,				0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"an open cage",			"the interior of the cage is filthy and reeks of decay."},
	{WALL_CHAR,		&gray,					&darkGray,				17, 0,	0,				0,			DF_MONSTER_CAGE_OPENS,	0,		NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_PROMOTES_WITH_KEY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),"a locked iron cage","the bars of the cage are firmly set and will not budge."},
	{ALTAR_CHAR,	&bridgeFrontColor,		&bridgeBackColor,		17,	20,	DF_COFFIN_BURNS,0,			DF_COFFIN_BURSTS,0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),											"a sealed coffin",		"a coffin made from thick wooden planks rests in a bed of bloodstained moss."},
	{ALTAR_CHAR,	&black,					&bridgeBackColor,		17,	20,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),										"an empty coffin",		"an open wooden coffin rests in a bed of bloodstained moss."},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			DF_ALEMBIC_POTION_CLOSES, 0,	CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"an empty alembic",		"an empty alembic. You could put a potion in here.",		(T2_PROMOTES_WITH_POTION)},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),													"a used alembic",		"an used alembic. It is burned and discoloured."},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			DF_CONTAINER_DARTS_OPENS, 0,	NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),	"a crystal container",	"a crystal container filled with darts."},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			0,				0,				CRYSTAL_WALL_LIGHT,(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),													"a crystal container",	"a glass container sprayed with color. The lid is ajar."},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			DF_PRESS_POTION, 0,	CANDLE_LIGHT,(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),							"an empty alembic",		"an empty alembic. You could put a potion in here.",		(T2_PROMOTES_WITH_POTION | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			DF_PRESS_POTION_BURN, 0,		CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"an empty alembic",		"an empty alembic. You could put a potion in here.",		(T2_PROMOTES_WITH_POTION | T2_CIRCUIT_BREAKER)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_POTION,	0,		CRYSTAL_WALL_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded alembic",	"an alembic, bubbling with color.",	(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_BURN_POTION,	0,		CRYSTAL_WALL_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded alembic",	"an alembic, bubbling with color.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{ALTAR_CHAR,	&wallCrystalColor,		&altarBackColor,		17,	0,	0,				0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE),													"a used alembic",		"an used alembic. It has a strong chemical smell."},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_SCROLL_CLOSES,0,		NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a printing press",		"an empty press. You could put a scroll in here.",			(T2_PROMOTES_WITH_SCROLL | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_SCROLL,0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a printing press",		"an empty press. You could put a scroll in here.",			(T2_PROMOTES_WITH_SCROLL | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_SCROLL_BURN,0,			NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a printing press",		"an empty press. You could put a scroll in here.",			(T2_PROMOTES_WITH_SCROLL | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_WEAPON,0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a weapon press",		"an empty press. You could put a weapon in here.",			(T2_PROMOTES_WITH_WEAPON | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_ARMOR, 0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"an armor press",		"an empty press. You could put a suit of armor in here.",	(T2_PROMOTES_WITH_ARMOR | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_SHIELD, 0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a shield press",		"an empty press. You could put a shield in here.",			(T2_PROMOTES_WITH_SHIELD | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_WAND,	0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a wand press",			"an empty press. You could put a wand in here.",			(T2_PROMOTES_WITH_WAND | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_STAFF, 0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a staff press",		"an empty press. You could put a staff in here.",			(T2_PROMOTES_WITH_STAFF | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_RUNE,	0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a rune press",			"an empty press. You could put a rune in here.",			(T2_PROMOTES_WITH_KEY_DROPPED | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_RUNE_WAND,	0,			NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a rune press",			"an empty press. You could put a rune in here.",			(T2_PROMOTES_WITH_KEY_DROPPED | T2_CIRCUIT_BREAKER)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			DF_PRESS_RUNE_STAFF, 0,			NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a rune press",			"an empty press. You could put a rune in here.",			(T2_PROMOTES_WITH_KEY_DROPPED | T2_CIRCUIT_BREAKER)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_SCROLL,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_BURN_SCROLL,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_WEAPON,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_ARMOR,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_SHIELD,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_WAND,	0,		BRIMSTONE_FIRE_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_STAFF,	0,		LUMINESCENT_ALGAE_BLUE_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_RUNE,	0,		LAVA_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_RUNE_WAND,	0,	PIXIE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{WALL_CHAR,		&itemColor,				&altarBackColor,		17,	0,	0,				0,			DF_PRESS_STAMP_RUNE_STAFF,	0,	LUMINESCENT_ALGAE_GREEN_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),	"a loaded press",	"a press, loaded and ready for its counterpart.",			(T2_PROMOTES_WITH_CIRCUIT_BROKEN)},
	{ALTAR_CHAR,	&wallCrystalColor,		&obsidianBackColor,		17,	0,	0,				0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),			"a broken press",		"a broken press, twisted by the stresses of untold forces."},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_MAKE_SACRIFICE,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a staff sacrifice",	"a weathered stone altar with space to sacrifice a staff.",			(T2_PROMOTES_WITH_STAFF)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_MAKE_SACRIFICE,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a food sacrifice",		"a weathered stone altar with space to sacrifice food.",			(T2_PROMOTES_WITH_FOOD)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_MAKE_SACRIFICE,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a ring sacrifice",		"a weathered stone altar with space to sacrifice a ring.",			(T2_PROMOTES_WITH_RING)},
	{ALTAR_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_MAKE_SACRIFICE,0,			CANDLE_LIGHT,	(T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),								"a gem sacrifice",		"a weathered stone altar with space to sacrifice a lumenstone.",	(T2_PROMOTES_WITH_GEM)},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_SACRIFICE_REWARDED,0,		CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),	"an iron cage",	"you must sacrifice something to lift the cage."},
	{ALTAR_CHAR,	&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_SURFACE_EFFECTS),																		"an ashen altar",		"a ash covered altar. A sacrifice has been made here."},
	{WALL_CHAR,		&altarBackColor,		&veryDarkGray,			17, 0,	0,				0,			DF_ALTAR_CHARM,0,				CANDLE_LIGHT,	(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),	"an iron cage",	"you must sacrifice something to imbue the charm."},
	{ALTAR_CHAR,	&floorBackColor,		&veryDarkGray,			17, 0,	0,				0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"an open cage",			"the interior of the cage is filthy and reeks of decay."},
	{WALL_CHAR,		&gray,					&darkGray,				17, 0,	0,				0,			DF_ARENA_CAGE_OPENS,	0,		NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE | T_IS_WIRED),"a locked iron cage","the bars of the cage are firmly set and will not budge."},
	{WALL_CHAR,		&wallCrystalColor,		&wallCrystalColor,		0,	0,	0,				0,			DF_PRISM_SHATTERS,0,			CRYSTAL_WALL_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_STAND_IN_TILE | T_IS_WIRED),"a crystal prism", "You feel the crystal's glossy surface and admire the dancing lights beneath."},
	{ALTAR_CHAR,	&floorBackColor,		&veryDarkGray,			17, 0,	0,				0,			0,				0,				ECTOPLASM_LIGHT,(T_STAND_IN_TILE),																					"an open chamber",		"the interior of the chamber is bloody and clouded with steam."},
	{STATUE_CHAR,	&altarForeColor,		&altarBackColor,		17, 0,	0,				0,			DF_CHAMBER_OPENS, 0,			NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE | T_IS_WIRED),				"a brass chamber",		"the door of the chamber is sealed firmly shut from the inside.",			(T2_REFLECTS_BOLTS)},
	{WALL_CHAR,		&gray,					&veryDarkGray,			17, 0,	0,				0,			DF_CHARGE_CAPACITOR, 0,			NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE | T_IS_WIRED),				"a drained capacitor",	"a drained capacitor. It needs lightning to charge it.",					(T2_REFLECTS_BOLTS | T2_CONDUCTIVE_WALL | T2_PROMOTES_WITH_LIGHTNING | T2_CIRCUIT_BREAKER)},
	{WALL_CHAR,		&lightningColor,		&altarBackColor,		17, 0,	0,				0,			DF_DISCHARGE_CAPACITOR,	0,		SPARK_TURRET_LIGHT,(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),							"a charged capacitor",	"the room hums and your hair stands on end. You need to charge the rest.",  (T2_REFLECTS_BOLTS | T2_PROMOTES_WITH_CIRCUIT_BROKEN | T2_IS_SPARKING)},
	{WALL_CHAR,		&gray,					&veryDarkGray,			17, 0,	0,				0,			0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE),															"a discharged capacitor", "the capacitor has discharged its stored energies and is spent."},

	// traps (part of dungeon layer):
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_POISON_GAS_CLOUD, DF_SHOW_POISON_GAS_TRAP, 0, 0,			NO_LIGHT,		(T_IS_SECRET | T_IS_DF_TRAP),																		"the ground",			""},
	{TRAP_CHAR,		&centipedeColor,		0,                      30,	0,	DF_POISON_GAS_CLOUD, 0,		0,				0,				NO_LIGHT,		(T_IS_DF_TRAP),																						"a poison gas trap",	"there is a hidden pressure plate in the floor above a reserve of poisonous gas."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_POISON_GAS_CLOUD, DF_SHOW_TRAPDOOR,0,	0,				NO_LIGHT,		(T_IS_SECRET | T_AUTO_DESCENT),																		"the ground",			"you plunge through a hidden trap door!"},
	{CHASM_CHAR,	&chasmForeColor,		&black,					30,	0,	DF_POISON_GAS_CLOUD,0,      0,				0,				NO_LIGHT,		(T_AUTO_DESCENT),																					"a hole",				"you plunge through a hole in the ground!"},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	0,              DF_SHOW_PARALYSIS_GAS_TRAP, 0, 0,           NO_LIGHT,		(T_IS_SECRET | T_IS_DF_TRAP | T_IS_WIRED),                                                          "the ground",			""},
	{TRAP_CHAR,		&pink,					0,              		30,	0,	0,              0,          0,				0,				NO_LIGHT,		(T_IS_DF_TRAP | T_IS_WIRED),																		"a paralysis trigger",	"there is a hidden pressure plate in the floor."},
    {FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	DF_DISCOVER_PARALYSIS_VENT, DF_PARALYSIS_VENT_SPEW,0,NO_LIGHT,	(T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_IS_SECRET),											"the ground",			""},
	{VENT_CHAR,		&pink,                  0,              		30,	0,	DF_PLAIN_FIRE,	0,			DF_PARALYSIS_VENT_SPEW,0,		NO_LIGHT,		(T_IS_WIRED),                                                                                       "an inactive gas vent",	"A dormant gas vent is connected to a reserve of paralytic gas."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_CONFUSION_GAS_TRAP_CLOUD,DF_SHOW_CONFUSION_GAS_TRAP, 0,0,NO_LIGHT,		(T_IS_SECRET | T_IS_DF_TRAP),																		"the ground",			""},
	{TRAP_CHAR,		&confusionGasColor,		0,              		30,	0,	DF_CONFUSION_GAS_TRAP_CLOUD,0,	0,			0,				NO_LIGHT,		(T_IS_DF_TRAP),																						"a confusion trap",		"A hidden pressure plate accompanies a reserve of psychotropic gas."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_FLAMETHROWER,	0,	DF_FLAMETHROWER_TRAP_HIDDEN, 0,		NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION),															"the ground",			""},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_FLAMETHROWER,	DF_SHOW_FLAMETHROWER_TRAP, 0,	0,		NO_LIGHT,		(T_IS_SECRET | T_IS_DF_TRAP),																		"the ground",			""},
	{TRAP_CHAR,		&fireForeColor,			0,              		30,	0,	DF_FLAMETHROWER,	0,		0,				0,				NO_LIGHT,		(T_IS_DF_TRAP),																						"a fire trap",			"A hidden pressure plate is connected to a crude flamethrower mechanism."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_FLOOD,		DF_SHOW_FLOOD_TRAP, 0,		0,				NO_LIGHT,		(T_IS_SECRET | T_IS_DF_TRAP),																		"the ground",			""},
	{TRAP_CHAR,		&shallowWaterForeColor,	0,              		58,	0,	DF_FLOOD,		0,			0,				0,				NO_LIGHT,		(T_IS_DF_TRAP),																						"a flood trap",			"A hidden pressure plate is connected to floodgates in the walls and ceiling."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	DF_SHOW_POISON_GAS_VENT, DF_POISON_GAS_VENT_OPEN, 0, NO_LIGHT, (T_IS_WIRED | T_IS_SECRET | T_VANISHES_UPON_PROMOTION),											"the ground",			""},
	{VENT_CHAR,		&floorForeColor,		0,              		30,	0,	DF_PLAIN_FIRE,	0,			DF_POISON_GAS_VENT_OPEN,0,		NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION),															"an inactive gas vent",	"An inactive gas vent is hidden in a crevice in the ground."},
	{VENT_CHAR,		&floorForeColor,		0,              		30,	0,	DF_PLAIN_FIRE,	0,			DF_VENT_SPEW_POISON_GAS,10000,	NO_LIGHT,		0,																									"a gas vent",			"Clouds of caustic gas are wafting out of a hidden vent in the floor."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	DF_SHOW_METHANE_VENT, DF_METHANE_VENT_OPEN,0,NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION | T_IS_SECRET),												"the ground",			""},
	{VENT_CHAR,		&floorForeColor,		0,              		30,	0,	DF_PLAIN_FIRE,	0,			DF_METHANE_VENT_OPEN,0,			NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION),															"an inactive gas vent",	"An inactive gas vent is hidden in a crevice in the ground."},
	{VENT_CHAR,		&floorForeColor,		0,              		30,	15,	DF_EMBERS,		0,			DF_VENT_SPEW_METHANE,5000,		NO_LIGHT,		(T_IS_FLAMMABLE),																					"a gas vent",			"Clouds of explosive gas are wafting out of a hidden vent in the floor."},
	{VENT_CHAR,		&gray,					0,              		15,	15,	DF_EMBERS,		0,			DF_STEAM_PUFF,	250,			NO_LIGHT,		T_OBSTRUCTS_ITEMS,																					"a steam vent",			"A natural crevice in the floor periodically vents scalding gouts of steam."},
	{VENT_CHAR,		&gray,					0,              		15,	15,	DF_EMBERS,		0,			DF_WIND_PUFF,	250,			NO_LIGHT,		T_OBSTRUCTS_ITEMS,																					"a windy vent",			"A natural crevice in the floor periodically vents swirling winds."},
	{TRAP_CHAR,		&white,					&chasmEdgeBackColor,	15,	0,	0,				0,			DF_MACHINE_PRESSURE_PLATE_USED,0,NO_LIGHT,      (T_IS_DF_TRAP | T_IS_WIRED | T_PROMOTES_ON_STEP | T_VANISHES_UPON_PROMOTION),                       "a pressure plate",		"There is an exposed pressure plate here. A thrown item might trigger it."},
	{TRAP_CHAR,		&darkGray,				&chasmEdgeBackColor,	15,	0,	0,				0,			0,				0,				NO_LIGHT,		0,                                                                                                  "an inactive pressure plate", "This pressure plate has already been depressed."},
    {CHASM_CHAR,	&glyphColor,            0,                      42,	0,	0,              0,          DF_INACTIVE_GLYPH,0,			GLYPH_LIGHT_DIM,(T_PROMOTES_ON_PLAYER_ENTRY | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),								"a magical glyph",      "A strange glyph, engraved into the floor, flickers with magical light."},
    {CHASM_CHAR,	&glyphColor,            0,                      42,	0,	0,              0,          DF_ACTIVE_GLYPH,10000,			GLYPH_LIGHT_BRIGHT,(T_VANISHES_UPON_PROMOTION),                                                                     "a glowing glyph",      "A strange glyph, engraved into the floor, radiates magical light."},
	
	// liquid layer
	{LIQUID_CHAR,	&deepWaterForeColor,	&deepWaterBackColor,	40,	100,DF_STEAM_ACCUMULATION,	0,	0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_IS_DEEP_WATER | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),							"the murky waters",		"the current tugs you in all directions.", (T2_CONDUCTIVE_FLOOR | T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE), (T3_IS_FERTILE)}, // grows algae
	{0,				&shallowWaterForeColor,	&shallowWaterBackColor,	55,	0,	DF_STEAM_ACCUMULATION,	0,	0,				0,				NO_LIGHT,		(T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),															"shallow water",		"the water is cold and reaches your knees.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE)},
	{MUD_CHAR,		&mudForeColor,			&mudBackColor,			55,	0,	DF_PLAIN_FIRE,	0,			DF_METHANE_GAS_PUFF, 100,		NO_LIGHT,		(T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),															"a bog",				"you are knee-deep in thick, foul-smelling mud.", 0L, (T3_IS_FERTILE)}, // grows dead man's ear
	{CHASM_CHAR,	&chasmForeColor,		&black,					40,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_AUTO_DESCENT | T_STAND_IN_TILE),																	"a chasm",				"you plunge downward into the chasm!"},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"the brink of a chasm",	"chilly winds blow upward from the stygian depths."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			DF_SPREADABLE_COLLAPSE,0,		NO_LIGHT,		(T_IS_WIRED | T_VANISHES_UPON_PROMOTION),															"the ground",			""},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	45,	0,	DF_PLAIN_FIRE,	0,			DF_COLLAPSE_SPREADS,2500,		NO_LIGHT,		(T_VANISHES_UPON_PROMOTION),																		"the crumbling ground",	"cracks are appearing in the ground beneath your feet!"},
	{LIQUID_CHAR,	&fireForeColor,			&lavaBackColor,			40,	0,	DF_OBSIDIAN,	0,			0,				0,				LAVA_LIGHT,		(T_LAVA_INSTA_DEATH | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),										"lava",					"searing heat rises from the lava."},
	{LIQUID_CHAR,	&fireForeColor,			&lavaBackColor,			40,	0,	DF_OBSIDIAN,	0,			DF_RETRACTING_LAVA,	0,			LAVA_LIGHT,		(T_LAVA_INSTA_DEATH | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),"lava","searing heat rises from the lava."},
	{LIQUID_CHAR,	&fireForeColor,			&lavaBackColor,			40,	0,	DF_OBSIDIAN,	0,			DF_OBSIDIAN_WITH_STEAM,	-1500,	LAVA_LIGHT,		(T_LAVA_INSTA_DEATH | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),			"cooling lava",         "searing heat rises from the lava."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		90,	0,	DF_PLAIN_FIRE,	0,			0,				0,				SUN_LIGHT,		(T_STAND_IN_TILE),																					"a patch of sunlight",	"sunlight streams through cracks in the ceiling."},
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		90,	0,	DF_PLAIN_FIRE,	0,			0,				0,				DARKNESS_PATCH_LIGHT,	(0),																						"a patch of shadows",	"this area happens to be cloaked in shadows -- perhaps a safe place to hide."},
	{ASH_CHAR,		&brimstoneForeColor,	&brimstoneBackColor,	40, 100,DF_INERT_BRIMSTONE,	0,		DF_INERT_BRIMSTONE,	10,			NO_LIGHT,		(T_IS_FLAMMABLE | T_SPONTANEOUSLY_IGNITES),															"hissing brimstone",	"the jagged brimstone hisses and spits ominously as it crunches under your feet."},
	{ASH_CHAR,		&brimstoneForeColor,	&brimstoneBackColor,	40, 0,	DF_INERT_BRIMSTONE,	0,		DF_ACTIVE_BRIMSTONE, 800,		NO_LIGHT,		(T_SPONTANEOUSLY_IGNITES),																			"hissing brimstone",	"the jagged brimstone hisses and spits ominously as it crunches under your feet."},
	{FLOOR_CHAR,	&darkGray,				&obsidianBackColor,		50,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"the obsidian ground",	"the ground has fused into obsidian."},
	{FLOOR_CHAR,	&darkGray,				&obsidianBackColor,		50,	0,	0,				0,			DF_OBSIDIAN_CRACKING,400,		LAVA_LIGHT,		(T_SPONTANEOUSLY_IGNITES | T_VANISHES_UPON_PROMOTION),												"the obsidian shell",	"the lava has temporarily fused into obsidian."},
	{ASH_CHAR,		&brimstoneForeColor,	&obsidianBackColor,		40, 0,	0,				0,			DF_OBSIDIAN_RUPTURING,800,		LAVA_LIGHT,		(T_SPONTANEOUSLY_IGNITES | T_VANISHES_UPON_PROMOTION),												"cracking obsidian",	"the obsidian begins to break apart under your feet."},
	{FIRE_CHAR,		&brimstoneForeColor,	&obsidianBackColor,		40, 0,	0,				0,			DF_LAVA,		1200,			LAVA_LIGHT,		(T_CAUSES_FIERY_DAMAGE | T_VANISHES_UPON_PROMOTION),												"rupturing obsidian",	"chunks of obsidian sink into the lava beneath you."},
	{BRIDGE_CHAR,	&bridgeFrontColor,		&bridgeBackColor,		20,	50,	DF_BRIDGE_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),														"a rickety rope bridge","the rickety rope bridge creaks underfoot."},
	{BRIDGE_CHAR,	&bridgeFrontColor,		&bridgeBackColor,		20,	50,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),														"a rickety rope bridge","the rickety rope bridge is staked to the edge of the chasm."},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	20,	50,	DF_BRIDGE_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"a stone bridge",		"the narrow stone bridge winds precariously across the chasm."},
	{0,				&shallowWaterForeColor,	&shallowWaterBackColor,	60,	0,	DF_STEAM_ACCUMULATION,	0,	DF_SPREADABLE_WATER,0,			NO_LIGHT,		(T_ALLOWS_SUBMERGING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),					"shallow water",		"the water is cold and reaches your knees.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE)},
	{0,				&shallowWaterForeColor,	&shallowWaterBackColor,	60,	0,	DF_STEAM_ACCUMULATION,	0,	DF_WATER_SPREADS,2500,			NO_LIGHT,		(T_ALLOWS_SUBMERGING | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),								"shallow water",		"the water is cold and reaches your knees.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE)},
	{MUD_CHAR,		&mudForeColor,			&mudBackColor,			55,	0,	DF_PLAIN_FIRE,	0,			DF_MUD_ACTIVATE,0,				NO_LIGHT,		(T_ALLOWS_SUBMERGING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),					"a bog",				"you are knee-deep in thick, foul-smelling mud.", 0L, (T3_IS_FERTILE)}, // grows dead man's ear
		
	// surface layer
	{CHASM_CHAR,	&chasmForeColor,		&black,					15,	0,	DF_PLAIN_FIRE,	0,			DF_HOLE_DRAIN,	-1000,			NO_LIGHT,		(T_AUTO_DESCENT | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"a hole",				"you plunge downward into the hole!"},
    {CHASM_CHAR,	&chasmForeColor,		&black,					15,	0,	DF_PLAIN_FIRE,	0,			DF_HOLE_DRAIN,	-1000,			DESCENT_LIGHT,	(T_AUTO_DESCENT | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),                                     "a hole",				"you plunge downward into the hole!"},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	50,	0,	DF_PLAIN_FIRE,	0,			0,				-500,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION),																		"translucent ground",	"chilly gusts of air blow upward through the translucent floor."},
	{LIQUID_CHAR,	&deepWaterForeColor,	&deepWaterBackColor,	41,	100,DF_STEAM_ACCUMULATION,	0,	DF_FLOOD_DRAIN,	-200,			NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_DEEP_WATER | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE), "sloshing water", "roiling water floods the room.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE)},
	{0,				&shallowWaterForeColor,	&shallowWaterBackColor,	50,	0,	DF_STEAM_ACCUMULATION,	0,	DF_PUDDLE,		-100,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),								"shallow water",		"knee-deep water drains slowly into holes in the floor.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING | T2_EXTINGUISHES_FIRE)},
	{GRASS_CHAR,	&grassColor,			0,						60,	15,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"grass-like fungus",	"grass-like fungus crunches underfoot.", 0L, (T3_IS_FERTILE)}, // grows bloodwort
	{GRASS_CHAR,	&deadGrassColor,		0,						60,	40,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"withered fungus",		"dead fungus covers the ground.", 0L, (T3_IS_FERTILE)}, // grows stinkfruit
	{GRASS_CHAR,	&grayFungusColor,		0,						51,	10,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"withered fungus",		"groping tendrils of pale fungus rise from the muck.", 0L, (T3_IS_FERTILE)}, // grows dead man's ear
	{GRASS_CHAR,	&fungusColor,			0,						60,	10,	DF_PLAIN_FIRE,	0,			0,				0,				FUNGUS_LIGHT,	(T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"luminescent fungus",	"luminescent fungus casts a pale, eerie glow.", 0L, (T3_IS_FERTILE)}, // grows crimson cap
	{GRASS_CHAR,	&lichenColor,			0,						60,	50,	DF_PLAIN_FIRE,	0,			DF_LICHEN_GROW,	10000,			NO_LIGHT,		(T_CAUSES_POISON | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),					"deadly lichen",		"venomous barbs cover the quivering tendrils of this fast-growing lichen.", 0L, (T3_IS_FERTILE)}, // grows symbiotic fungus
	{GRASS_CHAR,	&hayColor,				&refuseBackColor,		51,	50,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),										"filthy hay",			"a pile of hay, matted with filth, has been arranged here as a makeshift bed.", 0L, (T3_IS_FERTILE)}, // grows witch hazel
	{FLOOR_CHAR,	&humanBloodColor,		0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pool of blood",		"the floor is splattered with blood."},
	{FLOOR_CHAR,	&insectBloodColor,		0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pool of green blood", "the floor is splattered with green blood."},
	{FLOOR_CHAR,	&poisonGasColor,		0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pool of purple blood", "the floor is splattered with purple blood."},
	{FLOOR_CHAR,	&acidBackColor,			0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		0,																									"the acid-flecked ground", "the floor is splattered with acid."},
	{FLOOR_CHAR,	&vomitColor,			0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a puddle of vomit",	"the floor is caked with vomit."},
	{FLOOR_CHAR,	&urineColor,			0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				100,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),														"a puddle of urine",	"a puddle of urine covers the ground."},
	{FLOOR_CHAR,	&white,					0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				UNICORN_POOP_LIGHT,(T_STAND_IN_TILE),																				"unicorn poop",			"a pile of lavender-scented unicorn poop sparkles with rainbow light."},
	{FLOOR_CHAR,	&wormColor,				0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pool of worm entrails", "worm viscera cover the ground."},
	{ASH_CHAR,		&ashForeColor,			0,						80,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pile of ashes",		"charcoal and ash crunch underfoot."},
	{ASH_CHAR,		&ashForeColor,			0,						87,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"burned carpet",		"the carpet has been scorched by an ancient fire."},
	{FLOOR_CHAR,	&shallowWaterBackColor,	0,						80,	20,	0,				0,			0,				100,			NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),										"a puddle of water",	"a puddle of water covers the ground.", (T2_CONDUCTIVE_FLOOR| T2_GROUND_LIGHTNING)},
	{BONES_CHAR,	&bonesForeColor,		0,						70,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pile of bones",		"unidentifiable bones, yellowed with age, litter the ground."},
	{BONES_CHAR,	&gray,					0,						70,	0,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pile of rubble",		"rocky rubble covers the ground."},
	{BONES_CHAR,	&mudBackColor,			&refuseBackColor,		50,	20,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE),																					"a pile of filthy effects","primitive tools, carvings and trinkets are strewn about the area."},
	{FLOOR_CHAR,	&ectoplasmColor,		0,						70,	0,	DF_PLAIN_FIRE,	0,			0,				0,				ECTOPLASM_LIGHT,(T_STAND_IN_TILE),																					"ectoplasmic residue",	"a thick, glowing substance has congealed on the ground."},
	{ASH_CHAR,		&fireForeColor,			0,						70,	0,	DF_PLAIN_FIRE,	0,			DF_ASH,			300,			EMBER_LIGHT,	(T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),														"sputtering embers",	"sputtering embers cover the ground."},
	{WEB_CHAR,		&white,					0,						19,	100,DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_ENTANGLES | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),						"a spiderweb",			"thick, sticky spiderwebs fill the area.", (T2_CAN_CLIMB)},
	{FOLIAGE_CHAR,	&foliageColor,			0,						45,	15,	DF_PLAIN_FIRE,	0,			DF_TRAMPLED_FOLIAGE, 0,			NO_LIGHT,		(T_OBSTRUCTS_VISION | T_PROMOTES_ON_STEP | T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE | T_STAND_IN_TILE),"dense foliage",   "dense foliage fills the area, thriving on what sunlight trickles in."}, // grows bloodwort
	{FOLIAGE_CHAR,	&deadFoliageColor,		0,						45,	80,	DF_PLAIN_FIRE,	0,			DF_SMALL_DEAD_GRASS, 0,			NO_LIGHT,		(T_OBSTRUCTS_VISION | T_PROMOTES_ON_STEP | T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE | T_STAND_IN_TILE),"dead foliage",    "the decaying husk of a fungal growth fills the area.", 0L, 0L}, // grows stinkfruit
	{TRAMPLED_FOLIAGE_CHAR,&foliageColor,	0,						60,	15,	DF_PLAIN_FIRE,	0,			DF_FOLIAGE_REGROW, 100,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE),														"trampled foliage",		"dense foliage fills the area, thriving on what sunlight trickles in."}, // grows bloodwort
	{FOLIAGE_CHAR,	&fungusForestLightColor,0,						45,	15,	DF_PLAIN_FIRE,	0,			DF_TRAMPLED_FUNGUS_FOREST, 0,	FUNGUS_FOREST_LIGHT,(T_OBSTRUCTS_VISION | T_PROMOTES_ON_STEP | T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE | T_STAND_IN_TILE),"a luminescent fungal forest", "luminescent fungal growth fills the area, groping upward from the rich soil."}, // grows crimson cap 
	{TRAMPLED_FOLIAGE_CHAR,&fungusForestLightColor,0,				60,	15,	DF_PLAIN_FIRE,	0,			DF_FUNGUS_FOREST_REGROW, 100,	FUNGUS_LIGHT,	(T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE),														"trampled fungal forest", "luminescent fungal growth fills the area, groping upward from the rich soil.", 0L, (T3_IS_FERTILE)}, // grows crimson cap
	{FOLIAGE_CHAR,	&lichenFungusColor,		0,						45,	15,	DF_TRAMPLED_CREEPER_FUNGUS,	0, DF_TRAMPLED_CREEPER_FUNGUS, 200,	FUNGUS_FOREST_LIGHT,(T_OBSTRUCTS_VISION | T_PROMOTES_ON_STEP | T_OBSTRUCTS_SURFACE_EFFECTS | T_VANISHES_UPON_PROMOTION | T_IS_FLAMMABLE | T_STAND_IN_TILE),"a creeping fungal forest", "a sickly fungal growth which lives in a symbiotic relationship with a deadly lichen."}, // grows symbiotic fungus
	{TRAMPLED_FOLIAGE_CHAR,&lichenFungusColor, 0,					50,	15,	0,				0,			DF_CREEPER_FUNGUS_REGROW, 200,	FUNGUS_LIGHT,	(T_VANISHES_UPON_PROMOTION | T_OBSTRUCTS_SURFACE_EFFECTS),											"trampled creeping fungus", "a sickly fungal growth which lives in a symbiotic relationship with a deadly lichen.", 0L, (T3_IS_FERTILE)}, // grows symbiotic fungus
	{WALL_CHAR,		&forceFieldColor,		&forceFieldColor,		0,	0,	0,				0,			0,				-200,			FORCEFIELD_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_DIAGONAL_MOVEMENT | T_OBSTRUCTS_GAS | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),		"a green crystal",		"The translucent green crystal is melting away in front of your eyes."},
	{CHAIN_TOP_LEFT,&gray,					0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the ceiling.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_BOTTOM_RIGHT, &gray,				0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the floor.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_TOP_RIGHT, &gray,				0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the ceiling.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_BOTTOM_LEFT, &gray,				0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the floor.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_TOP,		&gray,					0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the wall.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_BOTTOM,	&gray,					0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the wall.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_LEFT,	&gray,					0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the wall.", (T2_CONDUCTIVE_FLOOR)},
	{CHAIN_RIGHT,	&gray,					0,						20,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron manacle",		"a thick iron manacle is anchored to the wall.", (T2_CONDUCTIVE_FLOOR)},
	{0,				0,						0,						1,	0,	0,				0,			0,				10000,			PORTAL_ACTIVATE_LIGHT,(T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),												"blinding light",		"blinding light streams out of the archway."},
    {0,				0,						0,						100,0,	0,				0,			0,				10000,			GLYPH_LIGHT_BRIGHT,(T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),                                                   "a red glow",           "a red glow fills the area."},
	{WALL_CHAR,		&gray,					0,						19,	100,DF_BURNING_BUNDLE,0,		0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),			"a sticky bundle",		"thick, sticky spiderwebs are wrapped tight around something which dangles here.", (T2_TUNNELIZE_IGNORES_GRID)},
	{WALL_CHAR,		&gray,					0,						19,	100,DF_BURNING_BUNDLE,0,		0,				0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),			"a sticky bundle",		"thick, sticky spiderwebs are wrapped tight around something which dangles here.", (T2_TUNNELIZE_IGNORES_GRID)},
	{WALL_CHAR,		&gray,					0,						19,	100,0,				0,			DF_PLAIN_FIRE,	10000,			NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),				"a burning bundle",		"the bundle of webs burns with a thick, black smoke.",	(T2_PROMOTES_WITH_FIRE | T2_TUNNELIZE_IGNORES_GRID)},
	{WEB_CHAR,		&white,					0,						19,	100,DF_SPIDER_BURNS,0,			DF_SPIDER_CLIMBS,0,				NO_LIGHT,		(T_ENTANGLES | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),			"a spiderweb",			"thick, sticky spiderwebs fill the area.",	(T2_PROMOTES_WITH_FIRE | T2_CAN_CLIMB)},
	{BRIDGE_CHAR,	&gray,					0,						18,	0,	0,				0,			0,				0,				NO_LIGHT,		0,																									"an iron chain",		"a thick iron chain is suspended overhead.", (T2_CONDUCTIVE_FLOOR | T2_CAN_CLIMB)},
	
	// fire tiles
	{FIRE_CHAR,		&fireForeColor,			0,						10,	0,	0,				0,			DF_EMBERS,		500,			FIRE_LIGHT,		(T_IS_FIRE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),											"billowing flames",		"flames billow upward."},
	{FIRE_CHAR,		&fireForeColor,			0,						10,	0,	0,				0,			0,				2500,			BRIMSTONE_FIRE_LIGHT,(T_IS_FIRE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),										"sulfurous flames",		"sulfurous flames leap from the unstable bed of brimstone."},
	{FIRE_CHAR,		&fireForeColor,			0,						10,	0,	0,				0,			DF_OBSIDIAN,	5000,			FIRE_LIGHT,		(T_IS_FIRE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),											"clouds of infernal flame", "billowing infernal flames eat at the floor."},
	{FIRE_CHAR,		&fireForeColor,			0,						10,	0,	0,              0,			0,              8000,			FIRE_LIGHT,		(T_IS_FIRE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),											"a cloud of burning gas", "flammable gas fills the air with flame."},
	{FIRE_CHAR,		&yellow,				0,						10,	0,	0,				0,			0,              10000,			EXPLOSION_LIGHT,(T_IS_FIRE | T_CAUSES_EXPLOSIVE_DAMAGE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION),				"a violent explosion",	"the force of the explosion slams into you."},
	{FIRE_CHAR,		&white,					0,						10,	0,	0,				0,			0,				10000,			INCENDIARY_DART_LIGHT ,(T_IS_FIRE | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),									"a flash of fire",		"flames burst out of the incendiary dart."},
	
	// gas layer
	{	' ',		0,						&poisonGasColor,		35,	100,DF_GAS_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_GAS_DISSIPATES | T_CAUSES_DAMAGE | T_STAND_IN_TILE),							"a cloud of caustic gas", "you can feel the purple gas eating at your flesh."},
	{	' ',		0,						&confusionGasColor,		35,	100,DF_GAS_FIRE,	0,			0,				0,				CONFUSION_GAS_LIGHT,(T_IS_FLAMMABLE | T_GAS_DISSIPATES_QUICKLY | T_CAUSES_CONFUSION | T_STAND_IN_TILE),				"a cloud of confusion gas", "the rainbow-colored gas tickles your brain."},
	{	' ',		0,						&vomitColor,			35,	100,DF_GAS_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_GAS_DISSIPATES_QUICKLY | T_CAUSES_NAUSEA | T_STAND_IN_TILE),					"a cloud of putrescence", "the stench of rotting flesh is overpowering."},
	{	' ',		0,						&pink,					35,	100,DF_GAS_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_GAS_DISSIPATES_QUICKLY | T_CAUSES_PARALYSIS | T_STAND_IN_TILE),					"a cloud of paralytic gas", "the pale gas causes your muscles to stiffen."},
	{	' ',		0,						&methaneColor,			35,	100,DF_EXPLOSION_FIRE,0,		0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_STAND_IN_TILE),																	"a cloud of explosive gas",	"the smell of explosive swamp gas fills the air."},
	{	' ',		0,						&white,					35,	0,	DF_GAS_FIRE,	0,			0,				0,				NO_LIGHT,		(T_GAS_DISSIPATES_QUICKLY | T_STAND_IN_TILE | T_CAUSES_FIERY_DAMAGE),								"a cloud of scalding steam", "scalding steam fills the air!"},
	{	' ',		0,						0,						35,	0,	DF_GAS_FIRE,	0,			0,				0,				SUNLIGHT_CLOUD_LIGHT,	(T_STAND_IN_TILE),																			"a cloud of sun light", "dust motes float in the warm rays of light."},
	{	' ',		0,						0,						35,	0,	DF_GAS_FIRE,	0,			0,				0,				DARKNESS_CLOUD_LIGHT,	(T_STAND_IN_TILE),																			"a cloud of supernatural darkness", "everything is obscured by an aura of supernatural darkness."},
	{	' ',		0,						&darkRed,				35,	0,	DF_GAS_FIRE,	0,			0,				0,				NO_LIGHT,		(T_STAND_IN_TILE | T_GAS_DISSIPATES_QUICKLY),														"a cloud of healing spores", "bloodwort spores, renowned for their healing properties, fill the air.", 0L, (T3_CAUSES_HEALING)},
	{	' ',		0,						&windColor,				30,	0,	DF_PLAIN_FIRE,	0,			0,				1500,			NO_LIGHT,		(T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),														"swirling winds", "strong winds swirl around you, clutching at your clothes.",	0L, (T3_IS_WIND)},
	{	' ',		0,						&lightBlue,				30,	0,	0,				0,			0,				0,				NO_LIGHT,		(T_GAS_DISSIPATES_QUICKLY | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),							"a cloud of inert gas", "this inert gas sucks the air from nearby flames.", (T2_EXTINGUISHES_FIRE)},

	// witch hazel flower
	{FOLIAGE_CHAR,	&white,					0,						10, 20, DF_PLAIN_FIRE,	0,			DF_WITCH_HAZEL_PICKED,	0,		PIXIE_LIGHT,	(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_PROMOTES_ON_PLAYER_ENTRY),							"a witch hazel flower", "a delicate flower which releases a burst of charging energy when picked.", 0L, (T3_IS_PLANTED | T3_AVOID)},

	// mandrake roots
	{GRASS_CHAR,	&mandrakeRootColor,		0,						51,	10,	DF_MANDRAKE_ROOT_DIES,	0,	DF_MANDRAKE_ROOT_DIES, 0,		NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_PROMOTES_ON_PLAYER_ENTRY),				"mandrake roots",	"sensitive tubers which are crushed when you stand on them.", 0L, (T3_IS_FERTILE)},
	{GRASS_CHAR,	&humanBloodColor,		0,						51,	10,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),														"dead mandrake roots",	"once sensitive tubers now sticky with blood-like fluids.", 0L, (T3_IS_FERTILE)},
	{GEM_CHAR,		&mandrakeRootColor,		0,						10,	20,	DF_MANDRAKE_SAC_SPLITS, 0,	DF_MANDRAKE_SAC_SPLITS, 0,		NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),				"mandrake sac",		"a fluid filled sac with something quivering inside. It seems to know you.", 0L, (T3_IS_PLANTED | T3_PROMOTES_ON_SHOT | T3_AVOID)},
	{FOLIAGE_CHAR,	&humanBloodColor,		0,						45,	20,	DF_PLAIN_FIRE,	0,			0,				0,				NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),														"split sac",		"a ruptured sac of fluids, split asunder from the inside."},
	{GRASS_CHAR,	&mandrakeRootColor,		0,						51,	10,	DF_MANDRAKE_ROOT_DIES,	0,	DF_MANDRAKE_ROOT_DIES, 500,		NO_LIGHT,		(T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_PROMOTES_ON_PLAYER_ENTRY),				"mandrake roots",	"sensitive tubers which are crushed when you stand on them.", 0L, (T3_IS_FERTILE)},
	{GEM_CHAR,		&mandrakeRootColor,		0,						10,	20,	DF_MANDRAKE_SAC_SPLITS, 0,	DF_MANDRAKE_SAC_SPLITS, 1000,	NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION | T_IS_WIRED),				"mandrake sac",		"a fluid filled sac with something quivering inside. It seems to know you.", 0L, (T3_IS_PLANTED | T3_PROMOTES_ON_SHOT | T3_AVOID)},

	// stinkfruit bushes
	{FOLIAGE_CHAR,  &darkGreen,				0,						10, 20, DF_PLAIN_FIRE,  0,          DF_STINKFRUIT_PODS_GROW, 200,	NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE),															"a stinkfruit stalk",  "this tendrilled bush grows foul smelling fruit.", 0L, (T3_IS_PLANTED)},
	{GEM_CHAR,		&vomitColor,			0,						11, 20, DF_STINKFRUIT_POD_BURST,0,	DF_STINKFRUIT_POD_BURST, 0,		NO_LIGHT,		(T_OBSTRUCTS_VISION | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION | T_PROMOTES_ON_STEP), "a stinkfruit", "the stinkfruit bursts, releasing a foul smelling cloud of spores.", 0L, (T3_IS_PLANTED | T3_PROMOTES_ON_SHOT | T3_AVOID)},
	{FOLIAGE_CHAR,  &darkGreen,				0,						10, 20, DF_PLAIN_FIRE,  0,          0,						2000,	NO_LIGHT,		(T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),								"a stinkfruit stalk",  "this tendrilled bush looks like it is dying.", 0L, (T3_IS_PLANTED)},

	// bloodwort pods
	{FOLIAGE_CHAR,  &bloodflowerForeColor,  &bloodflowerBackColor,  10, 20, DF_PLAIN_FIRE,  0,          DF_BLOODFLOWER_PODS_GROW, 100,	NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE),															"a bloodwort stalk",  "this spindly plant grows seed pods famous for their healing properties.", 0L, (T3_IS_PLANTED)},
	{GEM_CHAR,		&bloodflowerPodForeColor, 0,                    11, 20, DF_BLOODFLOWER_POD_BURST,0, DF_BLOODFLOWER_POD_BURST, 0,	NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE | T_STAND_IN_TILE | T_VANISHES_UPON_PROMOTION | T_PROMOTES_ON_PLAYER_ENTRY), "a bloodwort pod", "the bloodwort seed pod bursts, releasing a cloud of healing spores.", 0L, (T3_IS_PLANTED | T3_PROMOTES_ON_SHOT | T3_AVOID)},
	{FOLIAGE_CHAR,  &bloodflowerForeColor,  &bloodflowerBackColor,  10, 20, DF_PLAIN_FIRE,  0,          0,						2000,	NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_IS_FLAMMABLE | T_VANISHES_UPON_PROMOTION),								"a bloodwort stalk",  "this spindly plant looks like it is dying.", 0L, (T3_IS_PLANTED)},

	// algae
	{FLOOR_CHAR,	&floorForeColor,		&floorBackColor,		95,	0,	DF_PLAIN_FIRE,	0,			DF_ALGAE_1,		100,			NO_LIGHT,		0,																									"the ground",			""},
	{LIQUID_CHAR,	&deepWaterForeColor,    &deepWaterBackColor,	40,	100,DF_STEAM_ACCUMULATION,	0,	DF_ALGAE_1,     500,			LUMINESCENT_ALGAE_BLUE_LIGHT,(T_IS_FLAMMABLE | T_IS_DEEP_WATER | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),			"luminescent waters",	"blooming algae fills the waters with a swirling luminescence.", (T2_EXTINGUISHES_FIRE), (T3_IS_PLANTED)},
	{LIQUID_CHAR,	&deepWaterForeColor,    &deepWaterBackColor,	39,	100,DF_STEAM_ACCUMULATION,	0,	DF_ALGAE_REVERT,300,			LUMINESCENT_ALGAE_GREEN_LIGHT,(T_IS_FLAMMABLE | T_IS_DEEP_WATER | T_ALLOWS_SUBMERGING | T_STAND_IN_TILE),			"luminescent waters",	"blooming algae fills the waters with a swirling luminescence.", (T2_EXTINGUISHES_FIRE), (T3_IS_PLANTED)},

	// extensible stone bridge    
	{CHASM_CHAR,	&chasmForeColor,		&black,					40,	0,	DF_PLAIN_FIRE,	0,			0,              0,              NO_LIGHT,		(T_AUTO_DESCENT | T_STAND_IN_TILE),                                                                 "a chasm",				"you plunge downward into the chasm!"},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	40,	0,	DF_PLAIN_FIRE,	0,			DF_BRIDGE_ACTIVATE,6000,        NO_LIGHT,		(T_VANISHES_UPON_PROMOTION),                                                                        "a stone bridge",		"the narrow stone bridge is extending across the chasm."},
	{FLOOR_CHAR,	&white,					&chasmEdgeBackColor,	80,	0,	DF_PLAIN_FIRE,	0,			DF_BRIDGE_ACTIVATE_ANNOUNCE,0,	NO_LIGHT,		(T_IS_WIRED),                                                                                       "the brink of a chasm",	"chilly winds blow upward from the stygian depths."},

	// rat trap
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_WALL_CRACK,  0,              NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_IS_WIRED | T_STAND_IN_TILE),				"a stone wall",			"The rough stone wall is firm and unyielding."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_WALL_SHATTER,500,			NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_VANISHES_UPON_PROMOTION | T_STAND_IN_TILE),                             "a cracking wall",		"Cracks are running ominously across the base of this rough stone wall."},

	// worm tunnels
	{0,				0,						0,						100,0,	0,				0,			DF_WORM_TUNNEL_MARKER_ACTIVE,0, NO_LIGHT,       (T_VANISHES_UPON_PROMOTION | T_IS_WIRED),                                                           "",                     ""},
	{0,				0,						0,						100,0,	0,				0,			DF_GRANITE_CRUMBLES,-2000,		NO_LIGHT,		(T_VANISHES_UPON_PROMOTION),                                                                        "a rough granite wall",	"The granite is split open with splinters of rock jutting out at odd angles."},
	{WALL_CHAR,		&wallForeColor,			&wallBackColor,			0,	0,	DF_PLAIN_FIRE,	0,			DF_WALL_SHATTER,0,				NO_LIGHT,		(T_OBSTRUCTS_EVERYTHING | T_STAND_IN_TILE | T_IS_WIRED | T_VANISHES_UPON_PROMOTION),				"a stone wall",			"The rough stone wall is firm and unyielding."},
};

#pragma mark Dungeon Feature definitions

// Features in the gas layer use the startprob as volume, ignore probdecr, and spawn in only a single point.
// Intercepts and slopes are in units of 0.01.
dungeonFeature dungeonFeatureCatalog[NUMBER_DUNGEON_FEATURES] = {
	// tileType					layer		start	decr	fl	txt	fCol fRad	propTerrain	subseqDF		
	{0}, // nothing
	{GRANITE,					DUNGEON,	80,		70,		DFF_CLEAR_OTHER_TERRAIN},
	{CRYSTAL_WALL,				DUNGEON,	200,	50,		DFF_CLEAR_OTHER_TERRAIN},
	{LUMINESCENT_FUNGUS,		SURFACE,	60,		8,		DFF_BLOCKED_BY_OTHER_LAYERS},
	{GRASS,						SURFACE,	75,		5,		DFF_BLOCKED_BY_OTHER_LAYERS},
	{DEAD_GRASS,				SURFACE,	75,		5,		DFF_BLOCKED_BY_OTHER_LAYERS,	"",	0,	0,		0,			DF_DEAD_FOLIAGE},
	{BONES,						SURFACE,	75,		23,		0},
	{RUBBLE,					SURFACE,	45,		23,		0},
	{FOLIAGE,					SURFACE,	100,	33,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	{FUNGUS_FOREST,				SURFACE,	100,	45,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	{DEAD_FOLIAGE,				SURFACE,	50,		30,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	
	// misc. liquids
	{SUNLIGHT_POOL,				LIQUID,		65,		6,		0},
	{DARKNESS_PATCH,			LIQUID,		65,		11,		0},
	
	// Dungeon features spawned during gameplay:
	
	// revealed secrets
	{DOOR,						DUNGEON,	0,		0,		0},
	{GAS_TRAP_POISON,			DUNGEON,	0,		0,		0},
	{GAS_TRAP_PARALYSIS,		DUNGEON,	0,		0,		0},
	{CHASM_EDGE,				LIQUID,		100,	100,	0},
	{TRAP_DOOR,					LIQUID,		0,		0,		DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
	{GAS_TRAP_CONFUSION,		DUNGEON,	0,		0,		0},
	{FLAMETHROWER,				DUNGEON,	0,		0,		0},
	{FLOOD_TRAP,				DUNGEON,	0,		0,		0},
	
	// bloods
	// Start probability is actually a percentage for bloods.
	// Base probability is 15 + (damage * 2/3), and then take the given percentage of that.
	// If it's a gas, we multiply the base by an additional 100.
	// Thus to get a starting gas volume of a poison potion (1000), with a hit for 10 damage, use a starting probability of 48.
	{RED_BLOOD,					SURFACE,	100,	25,		0},
	{GREEN_BLOOD,				SURFACE,	100,	25,		0},
	{PURPLE_BLOOD,				SURFACE,	100,	25,		0},
	{WORM_BLOOD,				SURFACE,	100,	25,		0},
	{ACID_SPLATTER,				SURFACE,	200,	25,		0},
	{ASH,						SURFACE,	50,		25,		0},
	{EMBERS,					SURFACE,	125,	25,		0},
	{ECTOPLASM,					SURFACE,	110,	25,		0},
	{RUBBLE,					SURFACE,	33,		25,		0},
	{ROT_GAS,					GAS,		12,		0,		0},
	
	// monster effects
	{VOMIT,						SURFACE,	30,		10,		0},
	{POISON_GAS,				GAS,		2000,	0,		0},
	{GAS_EXPLOSION,				SURFACE,	350,	100,	0,	"",	&darkOrange, 4},
	{RED_BLOOD,					SURFACE,	150,	30,		0},
	{FLAMEDANCER_FIRE,			SURFACE,	200,	75,		0},
	
	// misc
	{NOTHING,					GAS,		0,		0,		DFF_EVACUATE_CREATURES_FIRST},
	{ROT_GAS,					GAS,		15,		0,		0},
	{STEAM,						GAS,		325,	0,		0},
	{WIND,						GAS,		325,	0,		0},
	{STEAM,						GAS,		15,		0,		0},
	{METHANE_GAS,				GAS,		2,		0,		0},
	{EMBERS,					SURFACE,	0,		0,		0},
	{URINE,						SURFACE,	65,		25,		0},
	{UNICORN_POOP,				SURFACE,	65,		40,		0},
	{PUDDLE,					SURFACE,	13,		25,		0},
	{ASH,						SURFACE,	0,		0,		0},
	{ECTOPLASM,					SURFACE,	0,		0,		0},
	{FORCEFIELD,				SURFACE,	100,	50,		0},
	{LICHEN,					SURFACE,	2,		100,	(DFF_BLOCKED_BY_OTHER_LAYERS)}, // Lichen won't spread through lava.
	{RUBBLE,					SURFACE,	45,		23,		(DFF_ACTIVATE_DORMANT_MONSTER)},
	
	// foliage
	{TRAMPLED_FOLIAGE,			SURFACE,	0,		0,		0},
	{DEAD_GRASS,				SURFACE,	75,		75,		0},
	{FOLIAGE,					SURFACE,	0,		0,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	{TRAMPLED_FUNGUS_FOREST,	SURFACE,	0,		0,		0},
	{FUNGUS_FOREST,				SURFACE,	0,		0,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	{TRAMPLED_CREEPER_FUNGUS,	SURFACE,	0,		0,		(DFF_SUBSEQ_EVERYWHERE),	"the fungus releases a cloud of spores",	0,	0,		0,			DF_CREEPER_LICHEN_CLOUD},
	{CREEPER_FUNGUS,			SURFACE,	0,		0,		(DFF_BLOCKED_BY_OTHER_LAYERS)},
	{LICHEN,					SURFACE,	170,	60,		0},
	
	// brimstone
	{ACTIVE_BRIMSTONE,			LIQUID,		0,		0,		0},
	{INERT_BRIMSTONE,			LIQUID,		0,		0,		0,	"",	0,	0,		0,			DF_BRIMSTONE_FIRE},
	{INERT_BRIMSTONE,			LIQUID,		90,		60,		0},
	
	// witch hazel
	{HAY,						SURFACE,	0,		0,		(DFF_RECHARGE_STAFFS_BY_ONE), "a burst of energy is released when you pick the flower!", &pixieColor, 4},

	// mandrake roots
	{MANDRAKE_SAC_NATURE,		SURFACE,	0,		0,		0,	"a sac is implanted in the fungus",			0,	0,		0,			DF_MANDRAKE_ROOTS_GROW},
	{MANDRAKE_ROOT_NATURE,		SURFACE,	100,	45,		DFF_BLOCKED_BY_OTHER_LAYERS,  "", 0, 0, 0},
	{MANDRAKE_ROOT_MACHINE,		SURFACE,	100,	45,		DFF_BLOCKED_BY_OTHER_LAYERS,  "", 0, 0, 0, DF_LUMINESCENT_FUNGUS_BORDER},
	{LUMINESCENT_FUNGUS,		SURFACE,	100,	75,		DFF_BLOCKED_BY_OTHER_LAYERS},
	{DEAD_MANDRAKE_ROOT,		SURFACE,	0,		0,		0,	"",	0, 0, 0, DF_RED_BLOOD},
	{SPLIT_MANDRAKE_SAC,		SURFACE,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"the sac splits open and something slides out",	0, 0, 0, DF_RED_BLOOD},

	// stinkfruit bushes
	{STINKFRUIT_POD,			SURFACE,    60,     60,     DFF_EVACUATE_CREATURES_FIRST},
	{STINKFRUIT_POD,			SURFACE,    10,     10,     DFF_EVACUATE_CREATURES_FIRST},
	{ROT_GAS,					GAS,		300,	0,		0,	"",	&vomitColor,4},

	// bloodwort
	{BLOODFLOWER_POD,           SURFACE,    60,     60,     DFF_EVACUATE_CREATURES_FIRST},
	{BLOODFLOWER_POD,           SURFACE,    10,     10,     DFF_EVACUATE_CREATURES_FIRST},
	{HEALING_CLOUD,				GAS,		350,	0,		0},

	// algae
	{DEEP_WATER_ALGAE_WELL,     DUNGEON,    0,      0,      DFF_SUPERPRIORITY},
	{DEEP_WATER_ALGAE_1,		LIQUID,		50,		100,	0,  "", 0,  0,      DEEP_WATER, DF_ALGAE_2},
	{DEEP_WATER_ALGAE_2,        LIQUID,     0,      0,      0},
	{DEEP_WATER,                LIQUID,     0,      0,      DFF_SUPERPRIORITY},

	// doors, item cages, altars, glyphs, guardians -- reusable machine components
	{OPEN_DOOR,					DUNGEON,	0,		0,		0},
	{DOOR,						DUNGEON,	0,		0,		0},
	{OPEN_IRON_DOOR_INERT,		DUNGEON,	0,		0,		0},
	{OPEN_IRON_DOOR_ON,			DUNGEON,	0,		0,		0},
	{OPEN_IRON_DOOR_OFF,		DUNGEON,	0,		0,		0},
	{ALTAR_SWITCH_ON,			DUNGEON,	0,		0,		0},
	{ALTAR_SWITCH_OFF,			DUNGEON,	0,		0,		0},
	{ALTAR_SWITCH_OPEN,			DUNGEON,	0,		0,		0,	"the cages lift off of the remaining altars."},
	{ALTAR_SWITCH_CLOSED,		DUNGEON,	0,		0,		(DFF_EVACUATE_CREATURES_FIRST), "the cages lower to cover the altars."},
	{ALTAR_CAGE_OPEN,			DUNGEON,	0,		0,		0,	"the cages lift off of the altars as you approach."},
	{ALTAR_CAGE_CLOSED,			DUNGEON,	0,		0,		(DFF_EVACUATE_CREATURES_FIRST), "the cages lower to cover the altars."},
	{ALTAR_DOOR_OPEN,			DUNGEON,	0,		0,		0,	"the cages lift off of the altars as you step through the door."},
	{ALTAR_DOOR_CLOSED,			DUNGEON,	0,		0,		(DFF_EVACUATE_CREATURES_FIRST), "the cages lower to cover the altars."},
	{ALTAR_INERT,				DUNGEON,	0,		0,		(DFF_DETECT_ADOPTED_KEY), "you sense the location of the missing item!"},
	{ALTAR_INERT,				DUNGEON,	0,		0,		0},
	{FLOOR_FLOODABLE,			DUNGEON,	0,		0,		0,	"the altar retracts into the ground with a grinding sound."},
	{PORTAL_LIGHT,				SURFACE,	0,		0,		(DFF_EVACUATE_CREATURES_FIRST | DFF_ACTIVATE_DORMANT_MONSTER), "the archway flashes, and you catch a glimpse of another world!"},
    {MACHINE_GLYPH_INACTIVE,    DUNGEON,    0,      0,      0},
    {MACHINE_GLYPH,             DUNGEON,    0,      0,      0},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  ""},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  "the glyph beneath you glows, and the guardians take a step!"},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  "the mirrored totem flashes, reflecting the red glow of the glyph beneath you."},
    {MACHINE_GLYPH,             DUNGEON,    200,    95,     DFF_BLOCKED_BY_OTHER_LAYERS},
	{MACHINE_GLYPH,             DUNGEON,    100,    100,    DFF_BLOCKED_BY_OTHER_LAYERS},
    {WALL_LEVER,                DUNGEON,    0,      0,      0,  "you notice a lever hidden behind a loose stone in the wall."},
    {WALL_LEVER_PULLED,         DUNGEON,    0,      0,      0},
    {WALL_LEVER_HIDDEN,         DUNGEON,    0,      0,      0},
	
	// fire
	{PLAIN_FIRE,				SURFACE,	0,		0,		0},
	{GAS_FIRE,					SURFACE,	0,		0,		0},
	{GAS_EXPLOSION,				SURFACE,	60,		17,		0},
	{DART_EXPLOSION,			SURFACE,	0,		0,		0},
	{BRIMSTONE_FIRE,			SURFACE,	0,		0,		0},
	{CHASM,						LIQUID,		0,		0,		0,	"",	0,	0,		0,			DF_PLAIN_FIRE},
	{PLAIN_FIRE,				SURFACE,	100,	37,		0},
	{EMBERS,					SURFACE,	0,		0,		0},
	{EMBERS,					SURFACE,	100,	94,		0},
	{OBSIDIAN,					DUNGEON,	0,		0,		0},
	{OBSIDIAN_CRACKING,			DUNGEON,	0,		0,		0},
	{OBSIDIAN_RUPTURING,		DUNGEON,	0,		0,		0},
	{LAVA,						LIQUID,		0,		0,		0},

	// some traps
	{FLOOD_WATER_SHALLOW,		SURFACE,	225,	37,		DFF_LAVA_BECOMES_OBSIDIAN,	"",	0,	0,		0,			DF_FLOOD_2},
	{FLOOD_WATER_DEEP,			SURFACE,	175,	37,		DFF_LAVA_BECOMES_OBSIDIAN,	"the area is flooded as water rises through imperceptible holes in the ground."},
	{FLOOD_WATER_SHALLOW,		SURFACE,	10,		25,		DFF_LAVA_BECOMES_OBSIDIAN},
	{HOLE,						SURFACE,	200,	100,	0},
	{HOLE_EDGE,					SURFACE,	0,		0,		0},
	
	// gas trap effects
	{POISON_GAS,				GAS,		1000,	0,		0,	"a cloud of caustic gas sprays upward from the floor!"},
	{CONFUSION_GAS,				GAS,		300,	0,		0,	"a sparkling cloud of confusion gas sprays upward from the floor!"},
	{METHANE_GAS,				GAS,		10000,	0,		0}, // debugging toy
	
	// potions
	{POISON_GAS,				GAS,		1000,	0,		0,	"",	&poisonGasColor,4},
	{PARALYSIS_GAS,				GAS,		1000,	0,		0,	"",	&pink,4},
	{CONFUSION_GAS,				GAS,		1000,	0,		0,	"",	&confusionGasColor, 4},
	{PLAIN_FIRE,				SURFACE,	100,	37,		0,	"",	&darkOrange,4},
	{SUNLIGHT_CLOUD,			GAS,		1000,	0,		0},
	{DARKNESS_CLOUD,			GAS,		200,	0,		0},
	{HOLE_EDGE,					SURFACE,	300,	100,	0,	"",	&darkBlue,3,0,			DF_HOLE_2},
	{LICHEN,					SURFACE,	70,		60,		0},
	{FLOOD_WATER_DEEP,			SURFACE,	275,	37,		DFF_LAVA_BECOMES_OBSIDIAN, "",	&deepWaterForeColor,4},
	{WIND,						GAS,		1000,	0,		0,	"",	&windColor,4},
	{ROT_GAS,					GAS,		200,	0,		0,	"",	&vomitColor,4},
	{METHANE_GAS,				GAS,		300,	0,		0,	"",	&methaneColor,4},
	{GAS_EXPLOSION,				SURFACE,	350,	100,	0,	"",	&darkOrange, 4},
	{HEALING_CLOUD,				GAS,		400,	0,		0,	"",	&darkRed, 4},
	{EXTINGUISH_GAS,			GAS,		400,	0,		0,	"",	&lightBlue, 4},
    
    // other items
    {PLAIN_FIRE,				SURFACE,	100,	45,		0,	"",	&yellow,3},
//	{DARKNESS_CLOUD,			GAS,		200,	0,		0}, // staff/wand of phantoms
	{FLOOD_WATER_SHALLOW,		SURFACE,	150,	50,		DFF_LAVA_BECOMES_OBSIDIAN,	"",	&deepWaterForeColor,4}, // staff/wand of nagas
	{SPIDERWEB,					SURFACE,	15,		12,		0}, // staff/wand of spiders
	{BRIDGE,					SURFACE,	15,		12,		0}, // wand of nature over chasm
	{DEEP_WATER_ALGAE_WELL,     DUNGEON,    150,    100,	(DFF_SUPERPRIORITY | DFF_SUBSEQ_EVERYWHERE),	"",	0,	0,		DEEP_WATER,			DF_ALGAE_1},	// bolt of nature on water
	{GAS_EXPLOSION,				SURFACE,	350,	100,	0,	"The corpse detonates with terrifying force!",	&darkOrange, 4}, // bolt of detonation
	{METHANE_GAS,				GAS,		12,		0,		0},	// bolt of detonation hits zombie
	{DARKNESS_CLOUD,			GAS,		100,	0,		0}, // darkness dart
	{GRAPPLE_CHAIN,				SURFACE,	0,		0,		0}, // grappling dart
	{EXTINGUISH_GAS,			GAS,		400,	0,		0}, // extinguishing dart

	// machine components
	
	// coffin bursts open to reveal vampire:
	{COFFIN_OPEN,				DUNGEON,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"the coffin opens and a dark figure rises!", &darkGray, 3, 0, 0},
	{PLAIN_FIRE,				SURFACE,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"as flames begin to lick the coffin, its tenant bursts forth!", 0, 0, 0, DF_EMBERS_PATCH},
	{MACHINE_TRIGGER_FLOOR,		DUNGEON,	200,	100,	0},

	// gamble with the gods
	{ADD_OBSIDIAN_WALL,			DUNGEON,	250,	100,	(DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, DF_BONES},
	{ALTAR_INERT,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM | DFF_ALWAYS_MESSAGE),	"the gods give you insight!"},
	{ALTAR_INERT,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM | DFF_ALWAYS_MESSAGE),	"the gods grant you a boon!"},
	{ALTAR_INERT,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM | DFF_ALWAYS_MESSAGE),	"the gods frown upon you!"},
	{ALTAR_ITEM_GAMBLED,		DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_ALWAYS_MESSAGE),	"you have gambled with the gods."},
	{0, 0, 0, 0, (0), "graffiti here reads: 'gift 4/7, taken 2/7, burn 1/7'"},
	{0, 0, 0, 0, (0), "graffiti here reads: 'bless 2/7 reveal 2/7 bad 3/7'"},
	{0, 0, 0, 0, (0), "graffiti here reads: 'enchant 1/7, negate 1/7'"},
	{0, 0, 0, 0, (0), "graffiti here reads: 'plenty 1/7, descend 1/7'"},

	// coat darts
	{ALEMBIC_POTION_BURNED,		DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM),"the potion is drained through tiny tubes."},
	{CONTAINER_DARTS_OPEN,		DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the darts are sprayed with a fine mist coating."},

	// make elixir
	{ALEMBIC_POTION_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the potion is poured into the alembic."},
	{ALEMBIC_POTION_BURN_LOADED,DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the potion is poured into the alembic."},
	{ALEMBIC_POTION_USED,		DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the potion is distilled and combined."},
	{ALEMBIC_POTION_BURNED,		DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM),"a chemical smell fills the air."},

	// make charm
	{PRESS_USED,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM),"the scroll is erased, line by line."},
	{ALTAR_INERT,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS),				"the charm is imbued with power."},

	// press tomes
	{PRESS_SCROLL_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the scroll is loaded into the press."},
	{PRESS_SCROLL_BURN_LOADED,	DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the scroll is loaded into the press."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the scroll is pressed into a plate."},
	{PRESS_USED,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM),"the air fills with a burning smell."},

	// press runes
	{PRESS_WEAPON_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the weapon is loaded into the press."},
	{PRESS_ARMOR_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the armor is loaded into the press."},
	{PRESS_SHIELD_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the shield is loaded into the press."},
	{PRESS_ARMOR_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the wand is loaded into the press."},
	{PRESS_SHIELD_LOADED,		DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the staff is loaded into the press."},
	{PRESS_RUNE_LOADED,			DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the rune is loaded into the press."},
	{PRESS_RUNE_WAND_LOADED,	DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the rune is loaded into the press."},
	{PRESS_RUNE_STAFF_LOADED,	DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the rune is loaded into the press."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the weapon is marked with the radiant rune."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the armor is stamped with the burning rune."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the shield is stamped with the burning rune."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the wand is stamped with the burning rune."},
	{PRESS_USED,				DUNGEON,	0,		0,		DFF_MERGE_ITEMS,				"the staff is stamped with the glowing rune."},
	{PRESS_USED,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM), "the rune is superheated in a lava bath."},
	{PRESS_USED,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM), "the rune is superheated in a brimstone chamber."},
	{PRESS_USED,				DUNGEON,	0,		0,		(DFF_MERGE_ITEMS | DFF_DESTROY_ITEM), "the rune is supercharged in an algae solution."},

	// sacrifice items
	{ALTAR_SACRIFICED,			DUNGEON,	0,		0,		DFF_DESTROY_ITEM,				"you have made a sacrifice."},
	{ALTAR_INERT,				DUNGEON,	0,		0,		0,	"the cage lifts off of the altar."},

	// flooding tutorial:
	{ALTAR_CAGE_INERT,			DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"the cage lowers to cover the altar."},

	// throwing tutorial:
	{ALTAR_INERT,				DUNGEON,	0,		0,		0,	"the cage lifts off of the altar."},
	{TRAP_DOOR,					LIQUID,		225,	100,	(DFF_CLEAR_OTHER_TERRAIN | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
	{LAVA,						LIQUID,		225,	100,	(DFF_CLEAR_OTHER_TERRAIN)},
	{MACHINE_PRESSURE_PLATE_USED,DUNGEON,   0,      0,      0},

	// rat trap:
	{RAT_TRAP_WALL_CRACKING,    DUNGEON,    0,      0,      0,  "a scratching sound emanates from the nearby walls!", 0, 0, 0, DF_RUBBLE},

	// wooden barricade at entrance:
	{WOODEN_BARRICADE,			DUNGEON,	0,		0,		0},
	{PLAIN_FIRE,				SURFACE,	0,		0,		0,	"flames quickly consume the wooden barricade."},
	
	// wooden barricade around altar:
	{ADD_WOODEN_BARRICADE,		DUNGEON,	150,	100,	(DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, DF_SMALL_DEAD_GRASS},
	
	// statues around altar:
	{STATUE_INERT,				DUNGEON,	100,	100,	(DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, DF_RUBBLE},

	// shallow water flood machine:
	{MACHINE_FLOOD_WATER_SPREADING,	LIQUID,	0,		0,		(DFF_ATTACK_SHALLOWS),	"you hear a heavy click, and the nearby water begins flooding the area!"},
	{SHALLOW_WATER,				LIQUID,		0,		0,		0},
	{MACHINE_FLOOD_WATER_SPREADING,LIQUID,	100,	100,	(DFF_ATTACK_SHALLOWS),	"",	0,	0,		FLOOR_FLOODABLE,			DF_SHALLOW_WATER},
	{MACHINE_FLOOD_WATER_DORMANT,LIQUID,	250,	100,	(DFF_TREAT_AS_BLOCKING), "", 0, 0, 0, DF_SPREADABLE_DEEP_WATER_POOL},
	{DEEP_WATER,				LIQUID,		90,		100,	(DFF_CLEAR_OTHER_TERRAIN | DFF_PERMIT_BLOCKING)},
	
	// unstable floor machine:
	{MACHINE_COLLAPSE_EDGE_SPREADING,LIQUID,0,		0,		0,	"you hear a deep rumbling noise as the floor begins to collapse!"},
	{CHASM,						LIQUID,		0,		0,		DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
	{MACHINE_COLLAPSE_EDGE_SPREADING,LIQUID,100,	100,	0,	"",	0,	0,	FLOOR_FLOODABLE,	DF_COLLAPSE},
	{MACHINE_COLLAPSE_EDGE_DORMANT,LIQUID,	0,		0,		0},
	
	// levitation bridge machine:
    {CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,LIQUID,100,    100,    0,  "", 0,  0,  CHASM_WITH_HIDDEN_BRIDGE,  DF_BRIDGE_APPEARS},
    {CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,LIQUID,100,    100,    0,  "a stone bridge extends from the floor with a grinding sound.", 0,  0,  CHASM_WITH_HIDDEN_BRIDGE,  DF_BRIDGE_APPEARS},
	{STONE_BRIDGE,				LIQUID,		0,		0,		0,	""},
	{MACHINE_CHASM_EDGE,        LIQUID,     100,	100,	0},
	
	// retracting lava pool:
	{LAVA_RETRACTABLE,          LIQUID,     100,    100,     0,  "", 0,  0,  LAVA},
	{LAVA_RETRACTING,			LIQUID,		0,		0,		0,	"hissing fills the air as the lava begins to cool."},
	{OBSIDIAN,					SURFACE,	0,		0,		0,	"",	0,	0,		0,			DF_STEAM_ACCUMULATION},
	
	// hidden poison vent machine:
	{MACHINE_POISON_GAS_VENT_DORMANT,DUNGEON,0,		0,		0,	"you notice an inactive gas vent hidden in a crevice of the floor."},
	{MACHINE_POISON_GAS_VENT,	DUNGEON,	0,		0,		0,	"deadly purple gas starts wafting out of hidden vents in the floor!"},
	{PORTCULLIS_CLOSED,			DUNGEON,	0,		0,		DFF_EVACUATE_CREATURES_FIRST,	"with a heavy mechanical sound, an iron portcullis falls from the ceiling!"},
	{PORTCULLIS_DORMANT,		DUNGEON,	0,		0,		0,  "the portcullis slowly rises from the ground into a slot in the ceiling."},
	{POISON_GAS,				GAS,		25,		0,		0},
	
	// hidden methane vent machine:
	{MACHINE_METHANE_VENT_DORMANT,DUNGEON,0,		0,		0,	"you notice an inactive gas vent hidden in a crevice of the floor."},
	{MACHINE_METHANE_VENT,		DUNGEON,	0,		0,		0,	"explosive methane gas starts wafting out of hidden vents in the floor!", 0, 0, 0, DF_VENT_SPEW_METHANE},
	{METHANE_GAS,				GAS,		60,		0,		0},
	{PILOT_LIGHT,				DUNGEON,	0,		0,		0,	"a torch falls from its mount and lies sputtering on the floor."},
    
    // paralysis trap:
	{MACHINE_PARALYSIS_VENT,    DUNGEON,    0,		0,		0,	"you notice an inactive gas vent hidden in a crevice of the floor."},
	{PARALYSIS_GAS,				GAS,		350,	0,		0,	"paralytic gas sprays upward from hidden vents in the floor!", 0, 0, 0, DF_REVEAL_PARALYSIS_VENT_SILENTLY},
	{MACHINE_PARALYSIS_VENT,    DUNGEON,    0,		0,		0},
    
    // thematic dungeon:
    {RED_BLOOD,					SURFACE,	75,     25,		0},
	
	// statuary:
	{STATUE_CRACKING,			DUNGEON,	0,		0,		0,	"cracks begin snaking across the marble surface of the statue!", 0, 0, 0, DF_RUBBLE},
	{RUBBLE,					SURFACE,	120,	100,	DFF_ACTIVATE_DORMANT_MONSTER,	"the statue shatters!", &darkGray, 3, 0, DF_RUBBLE},
	
	// hidden turrets:
	{TOP_WALL,					DUNGEON,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"you hear a click, and the stones in the wall shift to reveal turrets!", 0, 0, 0, DF_RUBBLE},
    
    // worm tunnels:
    {WORM_TUNNEL_MARKER_DORMANT,LIQUID,     5,      5,      0,  "", 0,  0,  GRANITE},
    {WORM_TUNNEL_MARKER_ACTIVE, LIQUID,     0,      0,      0},
    {FLOOR,                     DUNGEON,    0,      0,      (DFF_SUPERPRIORITY | DFF_ACTIVATE_DORMANT_MONSTER),  "", 0,  0,  0,  DF_TUNNELIZE},
	{FLOOR,                     DUNGEON,    0,      0,      0,  "the nearby wall cracks and collapses in a cloud of dust!", &darkGray,  5,  0,  DF_TUNNELIZE},
	
	// haunted room:
	{DARK_FLOOR_DARKENING,		DUNGEON,	0,		0,		0,	"the light in the room flickers and you feel a chill in the air."},
	{DARK_FLOOR,				DUNGEON,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"", 0, 0, 0, DF_ECTOPLASM_DROPLET},
    {HAUNTED_TORCH_TRANSITIONING,DUNGEON,   0,      0,      0},
    {HAUNTED_TORCH,             DUNGEON,    0,      0,      0},
	
	// mud pit:
	{MACHINE_MUD_DORMANT,		LIQUID,		100,	100,	0},
	{MUD,						LIQUID,		0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"across the bog, bubbles rise ominously from the mud."},
	
	// arena
	{ARENA_CAGE_OPEN,			DUNGEON,	0,		0,		DFF_ENRAGE_ALLIES},
	{FLAMETHROWER_HIDDEN,		DUNGEON,	0,		0,		0},

	// prism
	{FORCEFIELD,				DUNGEON,	100,	50,		0},
	{CRYSTAL_PRISM,				DUNGEON,	250,	100,	(DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, DF_RUBBLE},

	// chamber
	{CHAMBER_OPEN,				DUNGEON,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,   "the chamber shakes, sparks flying off it, and the door swings slowly open!"},
	{CAPACITOR_CHARGED,			DUNGEON,	0,		0,		0,   "the capacitor begins to hum loudly!"},
	{CAPACITOR_DISCHARGED,		DUNGEON,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER},

	// sticky bundles
	{BURNING_BUNDLE,			SURFACE,	50,		40,		DFF_ACTIVATE_DORMANT_MONSTER,	"as the sticky bundle begins to smoke, its contents begins to escape!", 0, 0, 0, DF_EMBERS_PATCH},
	{PLAIN_FIRE,				SURFACE,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"a burning spider tumbles from the ceiling!", 0, 0, 0, DF_EMBERS_PATCH},
	{SPIDERWEB,					SURFACE,	0,		0,		DFF_ACTIVATE_DORMANT_MONSTER,	"a spider lowers itself from the ceiling on a single silken strand!"},

	// idyll:
	{SHALLOW_WATER,				LIQUID,		250,	100,	(DFF_TREAT_AS_BLOCKING), "", 0, 0, 0, DF_DEEP_WATER_POOL},
	{DEEP_WATER,				LIQUID,		90,		100,	(DFF_CLEAR_OTHER_TERRAIN)},
	
	// swamp:
	{SHALLOW_WATER,				LIQUID,		20,		100,	0},
	{GRAY_FUNGUS,				SURFACE,	80,		50,		0,	"", 0, 0, 0, DF_SWAMP_MUD},
	{MUD,						LIQUID,		75,		5,		0,	"", 0, 0, 0, DF_SWAMP_WATER},
	
	// camp:
	{HAY,						SURFACE,	90,		87,		0},
	{JUNK,						SURFACE,	20,		20,		0},
	
	// remnants:
	{CARPET,					DUNGEON,	110,	20,		DFF_SUBSEQ_EVERYWHERE,	"", 0, 0, 0, DF_REMNANT_ASH},
	{BURNED_CARPET,				SURFACE,	120,	100,	0},
	
	// chasm catwalk:
	{CHASM,						LIQUID,		0,		0,		DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
	{STONE_BRIDGE,				LIQUID,		0,		0,		DFF_CLEAR_OTHER_TERRAIN},

	// lake catwalk:
	{DEEP_WATER,				LIQUID,		0,		0,		DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, DF_LAKE_HALO},
	{SHALLOW_WATER,				LIQUID,		160,	100,	0},
	
	// worms pop out of walls:
	{RUBBLE,					SURFACE,	120,	100,	DFF_ACTIVATE_DORMANT_MONSTER,	"the nearby wall explodes in a shower of stone fragments!", &darkGray, 3, 0, DF_RUBBLE},
	
	// monster cages open:
	{MONSTER_CAGE_OPEN,			DUNGEON,	0,		0,		0},
};

#pragma mark Lights

// radius is in units of 0.01
const lightSource lightCatalog[NUMBER_LIGHT_KINDS] = {
	//color					radius range			fade%	passThroughCreatures
	{0},																// NO_LIGHT
	{&minersLightColor,		{0, 0, 1},				35,		true},		// miners light
	{&fireBoltColor,		{300, 400, 1},			0,		false},		// burning creature light
	{&wispLightColor,		{400, 800, 1},			0,		false},		// will-o'-the-wisp light
	{&fireBoltColor,		{300, 400, 1},			0,		false},		// salamander glow
	{&pink,					{600, 600, 1},			0,		true},		// imp light
	{&pixieColor,			{400, 600, 1},			50,		false},		// pixie light
	{&lichLightColor,		{1500, 1500, 1},		0,		false},		// lich light
	{&flamedancerCoronaColor,{1000, 2000, 1},		0,		false},		// flamedancer light
	{&sentinelLightColor,	{300, 500, 1},			0,		false},		// sentinel light
	{&unicornLightColor,	{300, 400, 1},			0,		false},		// unicorn light
	{&ifritLightColor,		{300, 600, 1},			0,		false},		// ifrit light
	{&fireBoltColor,		{400, 600, 1},			0,		false},		// phoenix light
	{&fireBoltColor,		{150, 300, 1},			0,		false},		// phoenix egg light
	{&spectralBladeLightColor,{350, 350, 1},		0,		false},		// spectral blades
	{&summonedImageLightColor,{350, 350, 1},		0,		false},		// weapon images
	{&lightningColor,		{250, 250, 1},			35,		false},		// lightning turret light
	{&lightningColor,		{300, 300, 1},			0,		false},		// bolt glow
	{&telepathyColor,		{200, 200, 1},			0,		true},		// telepathy light
	{&markedColor,			{200, 200, 1},			0,		true},		// marked light
	{&lightBlue,			{300, 400, 1},			0,		false},		// reflective creature light
	
	// glowing terrain:
	{&torchLightColor,		{1000, 1000, 1},		50,		false},		// torch
	{&lavaLightColor,		{300, 300, 1},			50,		false},		// lava
	{&sunLightColor,		{200, 200, 1},			25,		true},		// sunlight
	{&darknessPatchColor,	{400, 400, 1},			0,		true},		// darkness patch
	{&fungusLightColor,		{300, 300, 1},			50,		false},		// luminescent fungus
	{&fungusForestLightColor,{500, 500, 1},			0,		false},		// luminescent forest
	{&algaeBlueLightColor,	{300, 300, 1},			0,		false},		// luminescent algae blue
	{&algaeGreenLightColor,	{300, 300, 1},			0,		false},		// luminescent algae green
	{&ectoplasmColor,		{200, 200, 1},			50,		false},		// ectoplasm
	{&unicornLightColor,	{200, 200, 1},			0,		false},		// unicorn poop light
	{&lavaLightColor,		{200, 200, 1},			50,		false},		// embers
	{&lavaLightColor,		{500, 1000, 1},			0,		false},		// fire
	{&lavaLightColor,		{200, 300, 1},			0,		false},		// brimstone fire
	{&explosionColor,		{DCOLS*100,DCOLS*100,1},100,	false},		// explosions
	{&dartFlashColor,		{15*100,15*100,1},		0,		false},		// incendiary darts
	{&portalActivateLightColor,	{DCOLS*100,DCOLS*100,1},0,	false},		// portal activation
	{&confusionLightColor,	{300, 300, 1},			100,	false},		// confusion gas
	{&sunLightCloudColor,	{250, 250, 1},			0,		true},		// sunlight cloud
	{&darknessCloudColor,	{500, 500, 1},			0,		true},		// darkness cloud
	{&forceFieldLightColor,	{200, 200, 1},			50,		false},		// forcefield
	{&crystalWallLightColor,{300, 500, 1},			50,		false},		// crystal wall
	{&torchLightColor,		{200, 400, 1},			0,		false},		// candle light
    {&hauntedTorchColor,	{400, 600, 1},          0,		false},		// haunted torch
    {&glyphLightColor,      {100, 100, 1},          0,      false},     // glyph dim light
    {&glyphLightColor,      {300, 300, 1},          0,      false},     // glyph bright light
    {&descentLightColor,    {600, 600, 1},          0,      false},     // magical pit light
};

#pragma mark Blueprints

const blueprint blueprintCatalog[NUMBER_BLUEPRINTS] = {
	{{0}}, // nothing
	//BLUEPRINTS:
	//depths			roomSize	freq	featureCt	flags	(features on subsequent lines)
	
		//FEATURES:
		//DF		terrain		layer		instanceCtRange	minInsts	itemCat		itemKind	itemValMin		monsterKind		reqSpace		hordeFl		itemFlags	featureFlags
	
	// -- REWARD ROOMS --

	// Mixed item library -- can check one item out at a time
	{{1, 12},           {30, 50},	30,		5,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{2,3},		3,			(WEAPON|ARMOR|WAND),-1,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{2,3},		2,			(STAFF|RING|CHARM),-1,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			STATUE_INERT,DUNGEON,		{2,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
	// Single item category library -- can check one item out at a time
	{{1, 15},           {30, 50},	15,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{3,4},		3,			(RING),		-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{4,5},		3,			(STAFF),	-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{3,4},		3,			(CHARM),	-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			STATUE_INERT,DUNGEON,		{2,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
	// Treasure room -- apothecary (potions)
	{{8, AMULET_LEVEL},	{20, 40},	10,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE | MF_ALTERNATIVE)},
		{0,			SHALLOW_WATER,LIQUID,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE | MF_ALTERNATIVE)},
		{0,			0,			0,				{5,7},		2,			(POTION),	-1,			0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING)},
		{0,			FUNGUS_FOREST,SURFACE,		{3,4},		0,			0,			-1,			0,				0,				2,				0,			0,			0},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			STATUE_INERT,DUNGEON,		{2,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
	// Treasure room -- archive (scrolls)
	{{8, AMULET_LEVEL},	{20, 40},	10,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,			0,				{4,6},		2,			(SCROLL),	-1,			100,			0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING)},
		{0,			FUNGUS_FOREST,SURFACE,		{3,4},		0,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE)},
		{0,			WIND_VENT,	 DUNGEON,		{3,4},		0,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			STATUE_INERT,DUNGEON,		{2,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
	// Guaranteed good permanent item on a glowing pedestal (runic weapon/armor, 2 staffs or 2 charms)
	{{5, AMULET_LEVEL},	{10, 30},	30,		8,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			STATUE_INERT,DUNGEON,		{2,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	-1,			500,			0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(ARMOR),	-1,			500,			0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(STAFF),	-1,			1000,			0,				2,				0,			(ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(CHARM),	-1,			0,              0,				2,				0,			(ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
	// Guaranteed good consumable item on glowing pedestals (scrolls of enchanting, potion of life, tomes, elixirs)
	{{10, AMULET_LEVEL},{10, 30},	30,		7,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(SCROLL),	SCROLL_ENCHANTING,0,		0,				2,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(POTION),	POTION_LIFE,0,              0,              2,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(TOME),		-1,			0,				0,				2,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(ELIXIR),	-1,			0,              0,              2,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
	// Outsourced item -- same item possibilities as in the good permanent item reward room, but directly adopted by 1-2 key machines.
	{{5, 17},           {0, 0},     20,		4,			(BP_REWARD | BP_NO_INTERIOR_FLAG),	{
		{0,			0,			0,				{1,1},		1,			(WEAPON),	-1,			500,			0,				0,				0,			(ITEM_IDENTIFIED | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			0,			0,				{1,1},		1,			(ARMOR),	-1,			500,			0,				0,				0,			(ITEM_IDENTIFIED | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			0,			0,				{2,2},		2,			(STAFF),	-1,			1000,			0,				0,				0,			(ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
		{0,			0,			0,				{2,2},		2,			(CHARM),	-1,			0,              0,				0,				0,			(ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
	// Dungeon -- two allies chained up for the taking
	{{5, AMULET_LEVEL},	{30, 80},	12,		6,			(BP_ROOM | BP_REWARD),	{
		{0,			VOMIT,		SURFACE,		{2,2},		2,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING)},
		{DF_AMBIENT_BLOOD,MANACLE_T,SURFACE,	{1,2},		1,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_AMBIENT_BLOOD,MANACLE_L,SURFACE,	{1,2},		1,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_BONES,	0,			0,				{2,3},		1,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_VOMIT,	0,			0,				{2,3},		1,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
	// Kennel -- allies locked in cages in an open room; choose one or two to unlock and take with you.
	{{5, AMULET_LEVEL},	{30, 80},	20,		5,			(BP_ROOM | BP_REWARD),	{
		{0,			MONSTER_CAGE_CLOSED,DUNGEON,{3,5},		3,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
		{0,			0,			0,				{1,2},		1,			KEY,		KEY_CAGE,	0,				0,				1,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_SKELETON_KEY | MF_KEY_DISPOSABLE)},
		{DF_AMBIENT_BLOOD, 0,	0,				{3,5},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_BONES,	0,			0,				{3,5},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			TORCH_WALL,	DUNGEON,		{2,3},		2,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)}}},
	// Vampire lair -- allies locked in cages and chained in a hidden room with a vampire in a coffin; vampire has one cage key.
	{{10, AMULET_LEVEL},{50, 80},	5,		4,			(BP_ROOM | BP_REWARD | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR),	{
		{DF_AMBIENT_BLOOD,0,	0,				{1,2},		1,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{DF_AMBIENT_BLOOD,MONSTER_CAGE_CLOSED,DUNGEON,{2,4},2,			0,			-1,			0,				0,				2,				(HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
		{DF_TRIGGER_AREA,COFFIN_CLOSED,0,		{1,1},		1,			KEY,		KEY_CAGE,	0,				MK_VAMPIRE,		1,				0,			(ITEM_IS_KEY),(MF_GENERATE_MONSTER | MF_GENERATE_ITEM | MF_SKELETON_KEY | MF_MONSTER_TAKE_ITEM | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN | MF_KEY_DISPOSABLE)},
		{DF_AMBIENT_BLOOD,SECRET_DOOR,DUNGEON,	{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
	// Legendary ally -- approach the altar with the crystal key to activate a portal and summon a legendary ally.
	{{8, AMULET_LEVEL},{30, 50},	15,		2,			(BP_ROOM | BP_REWARD),	{
		{DF_LUMINESCENT_FUNGUS,	ALTAR_KEYHOLE, DUNGEON,	{1,1}, 1,		KEY,		KEY_PORTAL,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_NEAR_ORIGIN | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)},
		{DF_LUMINESCENT_FUNGUS,	PORTAL,	DUNGEON,{1,1},		1,			0,			-1,			0,				0,				2,				HORDE_MACHINE_LEGENDARY_ALLY,0,	(MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},

	// Laboratory -- put a potion on the alembic which coats the stack of darts
	{{5, 12},			{20, 40},	20,		5,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALEMBIC_POTION_OPEN,DUNGEON,{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,			CONTAINER_DARTS_CLOSED,DUNGEON,{1,1},	1,			(WEAPON),	DART,		0,				0,				2,				0,			0,			(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Charm workshop -- put a potion on the alembic or scroll on the pedestal which transmutes a charm
	{{4, 15},		{20, 40},	10,			6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALEMBIC_POTION_OPEN,DUNGEON,{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE)},
		{0,			PRESS_SCROLL_OPEN,DUNGEON,{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE)},
		{0,			ALTAR_CHARM_LOADED,DUNGEON,	{1,1},		1,			(CHARM),	-1,			0,				0,				3,				0,			0,			(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Staff workshop -- put a rune and staff on a press to combine the two
	{{10, AMULET_LEVEL},{40, 70},	12,		4,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR |BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_STAFF_UNUSED,DUNGEON,{1,3},	1,			KEY,		KEY_RUNE_STAFF,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_STAFF_UNUSED,DUNGEON,	{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN)},
		{DF_BUILD_ALGAE_WELL,0,0,				{2, 3},		2,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,DEEP_WATER_ALGAE_1,LIQUID,			{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
	// Wand workshop -- put a rune and wand on a press to combine the two
	{{10, AMULET_LEVEL},{40, 70},	12,		4,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR |BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_WAND_UNUSED,DUNGEON,	{1,3},	1,			KEY,		KEY_RUNE_WAND,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_WAND_UNUSED,DUNGEON,	{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN)},
		{0,			INERT_BRIMSTONE,LIQUID,		{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},

	// -- CHALLENGE REWARD ROOMS --

	// Rune press -- put a rune and weapon on the press to combine the two.
	{{15, AMULET_LEVEL},{40, 70},	3,		4,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_UNUSED,DUNGEON,	{1,3},		1,			KEY,		KEY_RUNE_WEAPON,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_WEAPON_UNUSED,DUNGEON,{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN)},
		{0,			LAVA_RETRACTABLE,LIQUID,	{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{DF_CATWALK_BRIDGE,0,	0,				{0,0},		0,			0,			-1,			0,				0,				1,				0,			0,			(MF_EVERYWHERE)}}},
	// Rune press -- put a rune and armor or shield on the press to combine the two.
	{{15, AMULET_LEVEL},{40, 70},	6,		5,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR |BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_UNUSED,DUNGEON,	{1,3},		1,			KEY,		KEY_RUNE_ARMOR,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_ARMOR_UNUSED,DUNGEON,	{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN | MF_ALTERNATIVE)},
		{0,			PRESS_SHIELD_UNUSED,DUNGEON,{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN | MF_ALTERNATIVE)},
		{0,			LAVA_RETRACTABLE,LIQUID,	{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{DF_CATWALK_BRIDGE,0,	0,				{0,0},		0,			0,			-1,			0,				0,				1,				0,			0,			(MF_EVERYWHERE)}}},
	// Talisman library -- can check one item out at a time
	{{10, AMULET_LEVEL},{30, 50},	5,		3,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			VAULT_DOOR, DUNGEON,		{1,1},		1,			KEY,		KEY_VAULT,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),	(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{3,3},		3,			(TALISMAN),	-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING), (MF2_REQUIRE_NOT_NEGATIVE)}}},
	// Guaranteed very good item on a glowing pedestal. This version forces good runics onto items which don't normally get them.
	{{10, AMULET_LEVEL},{10, 30},	5,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	BROADSWORD,	0,				0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	WAR_AXE,	0,				0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	HAMMER,		0,				0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(ARMOR),	PLATE_MAIL,	0,				0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			VAULT_DOOR, DUNGEON,		{1,1},		1,			KEY,		KEY_VAULT,	0,				0,				1,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)}}},
	// Guaranteed very good item on a glowing pedestal. This version forces good runics onto javelins, or enchants weapons or armor higher than their normal starting amount.
	{{10, AMULET_LEVEL},{10, 30},	5,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	JAVELIN,	0,				0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(WEAPON),	0,			900,			0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{1,1},		1,			(ARMOR),	0,			900,			0,				2,				0,			ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_TREAT_AS_BLOCKING), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PEDESTAL,	DUNGEON,		{2,2},		2,			(SCROLL),	SCROLL_DUPLICATION,0,		0,				2,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
		{0,			VAULT_DOOR, DUNGEON,		{1,1},		1,			KEY,		KEY_VAULT,	0,				0,				1,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)}}},
	// Multi-rune staff workshop -- like staff workshop but already loaded with a runic staff
	{{15, AMULET_LEVEL},{40, 70},	3,		4,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_STAFF_UNUSED,DUNGEON,{1,3},	1,			KEY,		KEY_RUNE_STAFF,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_REQUIRE_GOOD_RUNIC), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		-1,			0,				0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_REQUIRE_GOOD_RUNIC), (MF2_FORCE_GOOD_RUNIC)},
		{DF_BUILD_ALGAE_WELL,0,0,				{2, 3},		2,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,DEEP_WATER_ALGAE_1,LIQUID,			{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
	// Multi-rune staff workshop -- like staff workshop but already loaded with a runic fire, lightning, force, poison, tunnelling or blinking staff
	{{15, AMULET_LEVEL},{40, 70},	3,		10,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_STAFF_UNUSED,DUNGEON,{1,3},	1,			KEY,		KEY_RUNE_STAFF,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE), (MF2_ONLY_ONE_FEATURE | MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_FIRE,	0,				0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_LIGHTNING,0,			0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_FORCE,0,				0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_POISON,0,				0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_TUNNELING,0,			0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_DETONATION,0,			0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{0,			PRESS_STAFF_LOADED,DUNGEON,	{1,1},		1,			STAFF,		STAFF_BLINKING,0,			0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_ALTERNATIVE), (MF2_FORCE_GOOD_RUNIC)},
		{DF_BUILD_ALGAE_WELL,0,0,				{2, 3},		2,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,DEEP_WATER_ALGAE_1,LIQUID,			{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
	// Multi-rune wand workshop -- like rune workshop but already loaded with a runic wand
	{{15, AMULET_LEVEL},{40, 70},	3,		3,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR |BP_PURGE_INTERIOR | BP_REWARD | BP_REQUIRE_CHALLENGE_ADOPT),	{
		{0,			PRESS_RUNE_WAND_UNUSED,DUNGEON,	{1,3},	1,			KEY,		KEY_RUNE_WAND,0,			0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_IDENTIFIED),(MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_REQUIRE_GOOD_RUNIC), (MF2_ONLY_ONE_FEATURE)},
		{0,			PRESS_WAND_LOADED,DUNGEON,	{1,1},		1,			WAND,		-1,			0,				0,				2,				0,			(ITEM_IDENTIFIED | ITEM_RUNIC_IDENTIFIED), (MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM | MF_REQUIRE_GOOD_RUNIC), (MF2_FORCE_GOOD_RUNIC)},
		{0,			INERT_BRIMSTONE,LIQUID,		{70, 70},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},

	// -- VESTIBULES --

	// Unforceable locked door, key guarded by an adoptive room
	{{1, AMULET_LEVEL},	{1, 1},     100,		1,		(BP_VESTIBULE),	{
		{0,			LOCKED_DOOR, DUNGEON,		{1,1},		1,			KEY,		KEY_DOOR,	0,				0,				1,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS), (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE | MF_IMPREGNABLE)}}},
	// Plain secret door
	{{2, AMULET_LEVEL},	{1, 1},     1,		1,			(BP_VESTIBULE),	{
		{0,			SECRET_DOOR, DUNGEON,		{1,1},		1,			0,          0,          0,				0,				1,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
	// Lever and either an exploding wall or a portcullis
	{{4, AMULET_LEVEL},	{1, 1},     10,		3,			(BP_VESTIBULE),	{
		{0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
		{0,			PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,			0,			0,			0,				0,				3,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
		{0,			WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
	// Flammable barricade in the doorway -- burn the wooden barricade to enter
	{{1, 6},			{1, 1},     8,		3,			(BP_VESTIBULE), {
		{0,			ADD_WOODEN_BARRICADE,DUNGEON,{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{0,			0,			0,				{1,1},		1,			WEAPON,		INCENDIARY_DART,0,			0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_INCINERATION,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
	// Statue in the doorway -- use a scroll of shattering to enter
	{{1, AMULET_LEVEL},	{1, 1},     6,		2,			(BP_VESTIBULE), {
		{0,			STATUE_INERT,DUNGEON,       {1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{0,			0,			0,				{1,1},		1,			SCROLL,		SCROLL_SHATTERING,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY)}}},
	// Statue in the doorway -- bursts to reveal monster
	{{5, AMULET_LEVEL},	{2, 2},     6,		2,			(BP_VESTIBULE), {
		{0,			STATUE_DORMANT,DUNGEON,		{1, 1},		1,			0,			-1,			0,				0,				1,				HORDE_MACHINE_STATUE,0,	(MF_PERMIT_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
		{0,			MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},	1,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)}}},
	// Throwing tutorial -- toss an item onto the pressure plate to retract the portcullis
	{{1, 4},			{70, 70},	10,     3,          (BP_VESTIBULE | BP_NO_INTERIOR_FLAG), {
		{DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,		0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,			0,			0,			0,				0,				3,				0,			0,			(MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
		{0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)}}},
	// Pit traps -- area outside entrance is full of pit traps
	{{1, AMULET_LEVEL},	{30, 60},	10,		3,			(BP_VESTIBULE | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
		{0,			DOOR,       DUNGEON,        {1,1},      1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
		{0,			SECRET_DOOR,DUNGEON,        {1,1},      1,			0,			0,			0,				0,				1,				0,			0,			(MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
		{0,			TRAP_DOOR_HIDDEN,DUNGEON,	{60, 60},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
	// Deep water -- just needs to be swum across
	{{2, AMULET_LEVEL},	{30, 60},	3,		2,			(BP_VESTIBULE | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
		{0,			DOOR,       DUNGEON,        {1,1},      1,			0,			0,			0,				0,				2,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{0,			DEEP_WATER,		LIQUID,		{60, 60},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
	// Beckoning obstacle -- a mirrored totem guards the door, and glyph are around the doorway.
	{{5, AMULET_LEVEL}, {15, 30},	10,		3,			(BP_VESTIBULE | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR), {
		{0,         DOOR,       DUNGEON,        {1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{0,         0,          0,              {1,1},		1,			0,			-1,			0,				MK_MIRRORED_TOTEM,3,			0,			0,			(MF_GENERATE_MONSTER | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN | MF_FAR_FROM_ORIGIN)},
		{0,         MACHINE_GLYPH,DUNGEON,      {1,1},		0,			0,			-1,			0,				0,				1,				0,			0,			(MF_NEAR_ORIGIN | MF_EVERYWHERE)},
		{0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
	// Guardian obstacle -- a guardian is in the door on a glyph, with other glyphs scattered around.
	{{6, AMULET_LEVEL}, {25, 25},	10,		4,          (BP_VESTIBULE | BP_OPEN_INTERIOR),	{
		{0,			DOOR,       DUNGEON,        {1,1},		1,			0,			0,			0,				MK_GUARDIAN,	2,				0,			0,			(MF_GENERATE_MONSTER | MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
		{0,			DOOR,       DUNGEON,        {1,1},		1,			0,			0,			0,				MK_WINGED_GUARDIAN,2,           0,			0,			(MF_GENERATE_MONSTER | MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
		{0,         MACHINE_GLYPH,DUNGEON,      {10,10},    3,          0,          -1,         0,              0,              1,              0,          0,          (MF_PERMIT_BLOCKING| MF_NEAR_ORIGIN)},
		{0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},

	// -- KEY HOLDERS --

	// Nested item library -- can check one item out at a time, and one is a disposable key to another reward room
	{{1, AMULET_LEVEL},	{30, 50},	35,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM_KEY | BP_IMPREGNABLE),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{1,2},		1,			(WEAPON|ARMOR|WAND),-1,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_NAMED | ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{1,2},		1,			(STAFF|RING|CHARM),-1,	0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_NAMED | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),	(MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),	(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)},
        {0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Here kitty kitty -- learn to kite monsters. Looks like a broken reward room.
	{{1, 4},		{30, 50},	10,		7,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM | BP_IMPREGNABLE),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			OPEN_IRON_DOOR_OFF, DUNGEON,{1,1},		0,			0,			-1,			0,				0,				2,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{0,			ALTAR_DOOR_CLOSED,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN | MF_ADOPT_ITEM)},
		{0,			ALTAR_DOOR_CLOSED,DUNGEON,	{2,4},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,			0,			0,              {2,2},		2,			(SCROLL),	SCROLL_AGGRAVATE_MONSTER,0,	0,				1,				0,			0,			(MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE | MF_GENERATE_ITEM)},
		{0,			0,			0,              {2,2},		2,			(SCROLL),	SCROLL_CAUSE_FEAR,0,		0,				1,				0,			0,			(MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE | MF_GENERATE_ITEM)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Do what I say, not what I do -- learn to interact with allies. Looks like a broken reward room.
	{{5, 9},		{30, 50},	5,		8,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM | BP_IMPREGNABLE),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			OPEN_IRON_DOOR_INERT, DUNGEON,{1,1},	0,			0,			-1,			0,				0,				2,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{0,			ALTAR_SWITCH_CLOSED,DUNGEON,{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_ADOPT_ITEM), (MF2_SET_AS_TARGET)},
		{0,			ALTAR_SWITCH_OFF,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING), (MF2_IN_VIEW_OF_TARGET | MF2_REQUIRE_ADJACENT_PASSABLE)},
		{0,			ALTAR_SWITCH_CLOSED,DUNGEON,{1,3},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,			0,			0,              {1,1},		1,			0,			-1,			0,				0,				1,				HORDE_LEADER_CAPTIVE,0,	(MF_BUILD_ANYWHERE_ON_LEVEL | MF_GENERATE_HORDE)},
		{0,			0,			0,              {2,2},		2,			(POTION),	POTION_PARALYSIS,0,			0,				1,				0,			0,			(MF_BUILD_ANYWHERE_ON_LEVEL | MF_GENERATE_ITEM)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// A moment of reflection -- key on an altar, room filled with pit, lightning turrets in walls, bridge appears when you charge the capacitors.
	{{11, AMULET_LEVEL}, {75, 120},	10,		7,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_CREATE_HALO)},
		{0,			CAPACITOR_UNCHARGED,DUNGEON,{2,3},		2,			0,			0,			0,				0,				2,				0,			0,			(MF_IN_VIEW_OF_ORIGIN | MF_NEAR_ORIGIN)},
		{0,			0,			0,				{2,3},		2,			0,			0,			0,				MK_SPARK_TURRET,1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN | MF_GENERATE_MONSTER)},
		{0,			0,			0,				{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM,LIQUID,{120, 120},1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_REMOVE_HALO)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,		0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
		{0,			0,			0,				{1,1},		1,			SHIELD,		-1,			0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
	// Flooding fun -- let the player learn what water does
	{{1, 4},			{40, 80},	10,		4,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
		{DF_SURROUND_STATUE,ALTAR_FLOODABLE,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET)},
		{0,			FLOOD_TRAP,DUNGEON,			{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE), (MF2_NEAR_TO_TARGET | MF2_IN_VIEW_OF_TARGET)},
		{0,			0,			0,				{2,2},		2,			POTION,		POTION_WATER,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Bombs are key -- manufacture a potion of explosive gas, then use it (made altar floodable for consistency)
	{{5, AMULET_LEVEL},			{40, 80},	5,		4,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
		{DF_SURROUND_STATUE,ALTAR_FLOODABLE,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{DF_SWAMP,	0,			0,				{4,4},		3,			0,			-1,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_WINDS,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			STATUE_INERT,DUNGEON,		{1,3},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)}}},
	// Pinball ride -- key on an altar, platforms with mirrored totems and glyphs.
	{{7, AMULET_LEVEL},	{75, 120},	5,	8,				(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET)},
		{DF_GLYPH_CIRCLE_SMALL,	0, 0,			{1,1},		1,			0,			0,			0,				MK_MIRRORED_TOTEM,3,			0,			0,			(MF_GENERATE_MONSTER | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_ADJACENT_TO_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_CREATE_HALO)},
		{0,			0,			0,				{9,9},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_ADJACENT_TO_EXITS)},
		{DF_GLYPH_CIRCLE_SMALL,	0, 0,			{9,9},		1,			0,			0,			0,				MK_MIRRORED_TOTEM,3,			0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS | MF_GENERATE_MONSTER | MF_TREAT_AS_BLOCKING), (MF2_NEAR_EXITS | MF2_IN_VIEW_OF_TARGET | MF2_MUST_VIEW_ADJACENT_GRIDS)},
		{DF_GLYPH_CIRCLE_SMALL,	0, 0,			{1,1},		1,			0,			0,			0,				MK_MIRRORED_TOTEM,2,			0,			0,			(MF_GENERATE_MONSTER | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_CLOSER_TO_ORIGIN_THAN_TARGET | MF2_BLINK_FROM_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_MUST_VIEW_ADJACENT_GRIDS)},
		{DF_GLYPH_CIRCLE_SMALL,	0, 0,			{9,9},		1,			0,			0,			0,				MK_MIRRORED_TOTEM,2,			0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS | MF_GENERATE_MONSTER | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_CLOSER_TO_ORIGIN_THAN_TARGET | MF2_BLINK_FROM_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_REQUIRE_ORIGIN_CANDIDATE)},
		{0,			MACHINE_GLYPH, DUNGEON,		{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)},
		{0,			CHASM,		LIQUID,			{120, 120},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE), (MF2_REMOVE_HALO)}}},
	// Grappling hook -- key on an altar, staff to sacrifice, designed to be navigated by staffs of spiders or blinking.
	{{7, AMULET_LEVEL},	{75, 120},	10,	11,				(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_CREATE_HALO)},
		{0,			0,			0,				{9,9},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_ADJACENT_TO_EXITS)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			0,			0,				{9,9},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS | MF_BUILD_IN_WALLS), (MF2_SET_AS_TARGET | MF2_CLOSER_TO_ORIGIN_THAN_TARGET | MF2_BLINK_FROM_TARGET | MF2_BLINK_TO_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_CAN_VIEW_ADJACENT_GRIDS)},
		{0,			0,			0,				{1,1},		1,			STAFF,		-1,			0,				0,				3,				0,			(ITEM_PLAYER_AVOIDS),(MF_BUILD_AT_ORIGIN | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			STAFF,		STAFF_BLINKING, 0,			0,				3,				0,			(ITEM_PLAYER_AVOIDS),(MF_BUILD_AT_ORIGIN | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			STAFF,		STAFF_SPIDER, 0,			0,				3,				0,			(ITEM_PLAYER_AVOIDS),(MF_BUILD_AT_ORIGIN | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_ALTERNATIVE)},
		{0,			0,			0,				{2,2},		2,			WEAPON,		GRAPPLING_DART,0,			0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			0,			0,			0,				0,				2,				0,			0,			(MF_NOT_IN_HALLWAY), (MF2_ADJACENT_TO_ORIGIN)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM,LIQUID,{120, 120},1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_REMOVE_HALO)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,		0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE)}}},
	// Cable guy -- key under cage, charge the capacitors by chaining them together.
	{{5, AMULET_LEVEL},	{75, 120},	5,	7,				(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET)},
		{0,			CAPACITOR_CHARGED, DUNGEON,	{9,9},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_ADJACENT_TO_TARGET | MF2_CREATE_HALO)},
		{0,			0,			0,				{9,9},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_ADJACENT_TO_EXITS)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			CAPACITOR_UNCHARGED, DUNGEON, {9,9},	1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_SET_AS_TARGET | MF2_CLOSER_TO_ORIGIN_THAN_TARGET | MF2_BLINK_FROM_TARGET | MF2_BLINK_TO_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_CAN_VIEW_ADJACENT_GRIDS)},
		{0,			0,			0,				{2,2},		2,			WEAPON,		GRAPPLING_DART,0,			0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY)},
		{0,			CHASM,		LIQUID,			{120, 120},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE), (MF2_REMOVE_HALO)}}},
	// Ionised solution - key under cage, charge capacitors by connecting them with water
	{{5, AMULET_LEVEL}, {35,80},	5,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR | BP_REQUIRE_CHALLENGE_PARENT),	{
		{0,			ALTAR_CAGE_RETRACTABLE,	DUNGEON,{1,1},	1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
		{0,			CAPACITOR_CHARGED,DUNGEON,	{1,1},		1,			0,			0,			0,				0,				4,				0,			0,			(MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET)},
		{0,			FLOOD_TRAP,DUNGEON,			{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING), (MF2_ADJACENT_TO_TARGET)},
		{0,			CAPACITOR_UNCHARGED,DUNGEON,{2,5},		2,			0,			0,			0,				0,				3,				0,			0,			(MF_TREAT_AS_BLOCKING), (MF2_IN_VIEW_OF_TARGET | MF2_BRIDGE_TO_TARGET)},
		{0,			0,			0,				{2,2},		2,			POTION,		POTION_WATER,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)}}},
	// Fiery forest - creeping lichen released by symbiotic fungus periodically burned away
	{{14, AMULET_LEVEL},	{50, 90},	5,	3,			(BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_TREAT_AS_BLOCKING),	{
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_CREATE_HALO)},
		{DF_BRIMSTONE_OUTCROP,0,0,				{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			0},
		{DF_CREEPER_LICHEN_CLOUD,CREEPER_FUNGUS, SURFACE, {4, 6}, 3,	0,			0,			0,				0,				1,				0,			0,			0},
		{0,			ASH,		SURFACE,		{1,1},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_EVERYWHERE)}}},

	// Secret room -- key on an altar in a secret room
	{{2, AMULET_LEVEL},	{15, 100},	1,		2,			(BP_ROOM | BP_ADOPT_ITEM), {
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			SECRET_DOOR,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
	// Throwing tutorial -- toss an item onto the pressure plate to retract the cage and reveal the key
	{{1, 4},			{70, 120},	10,		2,			(BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG), {
		{0,			ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
		{DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,		0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
	// Rat trap -- getting the key triggers paralysis vents nearby and also causes rats to burst out of the walls
	{{1,8},             {30, 70},	7,		3,          (BP_ADOPT_ITEM | BP_ROOM),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{1,1},1,		0,			-1,			0,				0,				2,				0,			0,			(MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)},
		{0,			RAT_TRAP_WALL_DORMANT,DUNGEON,{10,20},	5,			0,			-1,			0,				MK_RAT,         1,				0,			0,			(MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
	// Fun with fire -- trigger the fire trap and coax the fire over to the wooden barricade surrounding the altar and key
    {{3, 10},			{80, 100},	10,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
		{DF_SURROUND_WOODEN_BARRICADE,ALTAR_INERT,DUNGEON,{1,1},1,		0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{0,			GRASS,		SURFACE,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE | MF_ALTERNATIVE)},
		{DF_SWAMP,	0,			0,				{4,4},		2,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN)},
		{0,			FLAMETHROWER_HIDDEN,DUNGEON,{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NEAR_ORIGIN)},
		{0,			GAS_TRAP_POISON_HIDDEN,DUNGEON,{3, 3},	1,			0,			-1,			0,				0,				5,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE)},
		{0,			0,			0,				{2,2},		1,			POTION,		POTION_LICHEN,0,			0,				3,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
	// Flood room -- key on an altar in a room with pools of eel-infested waters; take key to flood room with shallow water
	{{3, AMULET_LEVEL},	{80, 180},	10,		4,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),	{
		{0,			FLOOR_FLOODABLE,LIQUID,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				5,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{DF_SPREADABLE_WATER_POOL,0,0,          {2, 4},		1,			0,			-1,			0,				0,				5,				HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
		{DF_GRASS,	FOLIAGE,	SURFACE,		{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0}}},
	// Fire trap room -- key on an altar, pools of water, fire traps all over the place.
	{{4, AMULET_LEVEL},	{80, 180},	10,		5,			(BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),	{
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,         0,          0,              {1, 1},     1,          0,          -1,         0,              0,              4,              0,          0,          MF_BUILD_AT_ORIGIN},
		{0,         FLAMETHROWER_HIDDEN,DUNGEON,{40, 60},   20,         0,          -1,         0,              0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING)},
		{DF_WATER_POOL,0,0,                     {4, 4},		1,			0,			-1,			0,				0,				4,				HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
		{DF_GRASS,	FOLIAGE,	SURFACE,		{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0}}},
	// Thief -- key stolen from altar by a thief
	{{1, AMULET_LEVEL},	{15, 20},	5,		4,			(BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG), {
		{DF_LUMINESCENT_FUNGUS,	ALTAR_STOLEN,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				1,				HORDE_MACHINE_THIEF,0,	(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_MONSTER_TAKE_ITEM | MF_GENERATE_HORDE | MF_NOT_IN_HALLWAY | MF_MONSTER_FLEEING)},
		{0,         STATUE_INERT,0,             {3, 5},     2,          0,          -1,         0,              0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
	// Collapsing floor area -- key on an altar in an area; take key to cause the floor of the area to collapse
	{{1, AMULET_LEVEL},	{45, 65},	13,		3,			(BP_ADOPT_ITEM | BP_TREAT_AS_BLOCKING),	{
		{0,			FLOOR_FLOODABLE,DUNGEON,	{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{DF_ADD_MACHINE_COLLAPSE_EDGE_DORMANT,0,0,{3, 3},	2,			0,			-1,			0,				0,				3,				0,			0,			(MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
	// Pit traps -- key on an altar, room full of pit traps
	{{1, AMULET_LEVEL},	{30, 100},	10,		3,			(BP_ROOM | BP_ADOPT_ITEM),	{
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{0,			TRAP_DOOR_HIDDEN,DUNGEON,	{30, 40},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{0,			SECRET_DOOR,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
	// Levitation challenge -- key on an altar, room filled with pit, levitation or lever elsewhere on level, bridge appears when you grab the key/lever.
	{{1, 13},			{75, 120},	10,		7,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_CREATE_HALO)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			0,			0,				{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM,LIQUID,{120, 120},1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_REMOVE_HALO)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,		0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_LEVITATION,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)},
        {0,			WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)}}},
	// Web climbing -- key on an altar, room filled with pit, spider at altar to shoot webs, bridge appears when you grab the key
	{{7, AMULET_LEVEL},	{55, 90},	10,		5,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				MK_SPIDER,		3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN | MF_GENERATE_MONSTER), (MF2_CREATE_HALO)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			0,			0,				{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			MF_BUILD_AT_ORIGIN},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM,LIQUID,	{120, 120},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS), (MF2_REMOVE_HALO)},
		{DF_ADD_DORMANT_CHASM_HALO,	CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE)}}},
	// Lava moat room -- key on an altar, room filled with lava, levitation/fire immunity/lever/water traps or water elsewhere on level, lava retracts when you grab the key/lever
	{{3, 13},			{75, 120},	12,		9,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_SET_AS_TARGET | MF2_CREATE_HALO)},
		{0,			0,			0,				{1,1},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_BUILD_AT_ORIGIN)},
		{0,			FLOOD_TRAP,DUNGEON,			{9,9},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS | MF_ALTERNATIVE), (MF2_SET_AS_TARGET | MF2_CLOSER_TO_ORIGIN_THAN_TARGET | MF2_BLINK_FROM_TARGET | MF2_BRIDGE_TO_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_REQUIRE_ORIGIN_CANDIDATE)},
		{0,			LAVA,       LIQUID,         {60,60},	1,			0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{DF_LAVA_RETRACTABLE, 0, 0,             {1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE), (MF2_REMOVE_HALO)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_LEVITATION,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_FIRE_IMMUNITY,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{2,2},		2,			POTION,		POTION_WATER,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)}}},
	// Lava moat area -- key on an altar, surrounded with lava, levitation/fire immunity/water elsewhere on level, lava retracts when you grab the key
	{{3, 13},			{40, 60},	3,		5,			(BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_TREAT_AS_BLOCKING),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING), (MF2_CREATE_HALO)},
		{0,			LAVA,       LIQUID,         {60,60},	1,			0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{DF_LAVA_RETRACTABLE, 0, 0,             {1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE), (MF2_REMOVE_HALO)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_LEVITATION,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_FIRE_IMMUNITY,0,		0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{2,2},		2,			POTION,		POTION_WATER,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
	// Poison gas -- key on an altar; take key to cause a poison gas vent to appear and the door to be blocked; there is a hidden trapdoor or an escape item somewhere inside
	{{4, AMULET_LEVEL},	{35, 60},	7,		7,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)},
		{0,			MACHINE_POISON_GAS_VENT_HIDDEN,DUNGEON,{1,2}, 1,	0,			-1,			0,				0,				2,				0,			0,			0},
		{0,			TRAP_DOOR_HIDDEN,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			MF_ALTERNATIVE},
		{0,			0,			0,				{1,1},		1,			SCROLL,		SCROLL_TELEPORT,0,			0,				2,				0,			0,			(MF_GENERATE_ITEM | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_DESCENT,0,			0,				2,				0,			0,			(MF_GENERATE_ITEM | MF_ALTERNATIVE)},
        {0,			WALL_LEVER_HIDDEN_DORMANT,DUNGEON,{1,1},1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
        {0,			PORTCULLIS_DORMANT,DUNGEON,{1,1},       1,          0,			0,			0,              0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
	// Explosive situation -- key on an altar; take key to cause a methane gas vent to appear and a pilot light to ignite
	{{7, AMULET_LEVEL},	{80, 90},	10,		5,			(BP_ROOM | BP_PURGE_LIQUIDS | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM),	{
		{0,			DOOR,       DUNGEON,		{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN)},
        {0,			FLOOR,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
        {0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
		{0,			MACHINE_METHANE_VENT_HIDDEN,DUNGEON,{1,1}, 1,		0,			-1,			0,				0,				1,				0,			0,			MF_NEAR_ORIGIN},
		{0,			PILOT_LIGHT_DORMANT,DUNGEON,{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_FAR_FROM_ORIGIN | MF_BUILD_IN_WALLS)}}},
	// Burning grass -- key on an altar; take key to cause pilot light to ignite grass in room
	{{1, 7},			{40, 110},	10,		6,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM | BP_OPEN_INTERIOR),	{
		{DF_SMALL_DEAD_GRASS,ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},1,	0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
		{DF_DEAD_FOLIAGE,0,		SURFACE,		{2,3},		0,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			FOLIAGE,	SURFACE,		{1,4},		0,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			GRASS,		SURFACE,		{10,25},	0,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			DEAD_GRASS,	SURFACE,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			PILOT_LIGHT_DORMANT,DUNGEON,{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			MF_NEAR_ORIGIN | MF_BUILD_IN_WALLS}}},
	// Statuary -- key on an altar, area full of statues; take key to cause statues to burst and reveal monsters
	{{10, AMULET_LEVEL},{35, 90},	10,		2,			(BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			STATUE_DORMANT,DUNGEON,		{3,5},		3,			0,			-1,			0,				0,				2,				HORDE_MACHINE_STATUE,0,	(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},
    // Guardian water puzzle -- key held by a guardian, flood trap in the room, glyphs scattered. Lure the guardian into the water to have him drop the key.
	{{4, AMULET_LEVEL}, {35, 70},	8,		4,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM),	{
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
		{0,			0,          0,              {1,1},		1,			0,			-1,			0,				MK_GUARDIAN,	2,				0,			0,			(MF_GENERATE_MONSTER | MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTER_TAKE_ITEM)},
		{0,			FLOOD_TRAP,DUNGEON,         {1,1},		1,			0,			-1,			0,				0,				2,				0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      4,          0,          -1,         0,              0,              2,              0,          0,          (MF_EVERYWHERE | MF_NOT_IN_HALLWAY)}}},
    // Guardian gauntlet -- key in a room full of guardians, glyphs scattered and unavoidable.
	{{6, AMULET_LEVEL}, {50, 95},	10,		6,			(BP_ROOM | BP_ADOPT_ITEM),	{
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
		{0,			DOOR,       DUNGEON,        {1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{0,			0,          0,              {3,6},		3,			0,			-1,			0,				MK_GUARDIAN,	2,				0,			0,			(MF_GENERATE_MONSTER | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,          0,              {1,2},		1,			0,			-1,			0,				MK_WINGED_GUARDIAN,2,           0,			0,			(MF_GENERATE_MONSTER | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {10,15},   10,          0,          -1,         0,              0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},
	// Don't forget your hat - as close as we can get to a boulder in a corridor
	{{4, AMULET_LEVEL},	{75, 90},	5,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS),        {
		{DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},		1,			0,			-1,			0,				MK_GUARDIAN,	3,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_GENERATE_MONSTER | MF_ALTERNATIVE)},
		{DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},		1,			0,			-1,			0,				MK_WINGED_GUARDIAN, 3,			0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_GENERATE_MONSTER | MF_ALTERNATIVE)},
		{DF_GLYPH_CIRCLE,0,		0,				{9,9},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_REPEAT_UNTIL_NO_PROGRESS | MF_TREAT_AS_BLOCKING), (MF2_ADJACENT_TO_EXITS)},
		{DF_GLYPH_CIRCLE,0,		0,				{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			0, (MF2_ADJACENT_TO_ORIGIN)},
		{0,			TOP_WALL,DUNGEON,			{90, 90},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{0,			MACHINE_GLYPH,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				0,				0,			0,			(MF_PERMIT_BLOCKING | MF_EVERYWHERE)}}},
	// Summoning circle -- key in a room with an eldritch totem, glyphs unavoidable. // DISABLED. (Not fun enough.)
	{{12, AMULET_LEVEL}, {50, 100},	0,		2,			(BP_ROOM | BP_OPEN_INTERIOR | BP_ADOPT_ITEM),	{
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
		{DF_GLYPH_CIRCLE,0,     0,              {1,1},		1,			0,			-1,			0,				MK_ELDRITCH_TOTEM,3,			0,			0,			(MF_GENERATE_MONSTER | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    // Beckoning obstacle -- key surrounded by glyphs in a room with a mirrored totem.
	{{5, AMULET_LEVEL}, {60, 100},	10,		4,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM), {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN | MF_IN_VIEW_OF_ORIGIN), (MF2_SET_AS_TARGET)},
		{0,         0,          0,              {1,1},		1,			0,			-1,			0,				MK_MIRRORED_TOTEM,3,			0,			0,			(MF_GENERATE_MONSTER | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_IN_VIEW_OF_TARGET | MF2_MUST_VIEW_ADJACENT_GRIDS)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
    // Worms in the walls -- step on trigger region to cause underworms to burst out of the walls
	{{12,AMULET_LEVEL},	{7, 7},		7,		2,			(BP_ADOPT_ITEM),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			WALL_MONSTER_DORMANT,DUNGEON,		{5,8},		5,			0,			-1,			0,				MK_UNDERWORM,	1,				0,			0,			(MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
	// Mud pit -- key on an altar, room full of mud, take key to cause bog monsters to spawn in the mud
	{{12, AMULET_LEVEL},{40, 90},	10,		3,			(BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS),	{
		{DF_SWAMP,		0,		0,				{5,5},		0,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_SWAMP,	ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{DF_MUD_DORMANT,0,		0,				{3,4},		3,			0,			-1,			0,				0,				1,				HORDE_MACHINE_MUD,0,	(MF_GENERATE_HORDE | MF_MONSTERS_DORMANT)}}},
	// Haunted house -- key on an altar; take key to cause the room to darken, ectoplasm to cover everything and phantoms to appear
	{{16, AMULET_LEVEL},{45, 150},	10,		4,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{0,			DARK_FLOOR_DORMANT,DUNGEON,	{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			DARK_FLOOR_DORMANT,DUNGEON,	{4,5},		4,			0,			-1,			0,				MK_PHANTOM,		1,				0,			0,			(MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT)},
        {0,         HAUNTED_TORCH_DORMANT,DUNGEON,{5,10},   3,          0,          -1,         0,              0,              2,              0,          0,          (MF_BUILD_IN_WALLS)}}},
    // Worm tunnels -- hidden lever causes tunnels to open up revealing worm areas and a key
    {{8, AMULET_LEVEL},{80, 175},	10,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_MAXIMIZE_INTERIOR | BP_SURROUND_WITH_WALLS),	{        
		{0,			ALTAR_INERT,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
		{0,			0,          0,              {3,6},		3,			0,			-1,			0,				MK_UNDERWORM,	1,				0,			0,			(MF_GENERATE_MONSTER)},
		{0,			GRANITE,    DUNGEON,        {150,150},  1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_WORM_TUNNEL_MARKER_DORMANT,GRANITE,DUNGEON,{0,0},0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE | MF_PERMIT_BLOCKING)},
        {DF_TUNNELIZE,WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,			WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    // Gauntlet -- key on an altar; take key to cause turrets to emerge
	{{5, 24},			{35, 90},	10,		2,			(BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),	{
		{0,			ALTAR_SWITCH,DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)},
		{0,			TURRET_DORMANT,DUNGEON,		{4,6},		4,			0,			-1,			0,				0,				2,				HORDE_MACHINE_TURRET,0,	(MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
	// Boss -- key is held by a boss atop a pile of bones in a secret room. A few fungus patches light up the area.
	{{5, AMULET_LEVEL},	{40, 100},	18,		3,			(BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS), {
		{DF_BONES,	SECRET_DOOR,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{DF_LUMINESCENT_FUNGUS,	STATUE_INERT,DUNGEON,{7,7},	0,			0,			-1,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{DF_BONES,	0,			0,				{1,1},		1,			0,			-1,			0,				0,				1,				HORDE_MACHINE_BOSS,	0,	(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_MONSTER_TAKE_ITEM | MF_GENERATE_HORDE | MF_MONSTER_SLEEPING)}}},

	// -- CHALLENGE KEY HOLDERS --

	// Arena - key is in cage. Lever opens all cages and enrages monsters in them, begins flooding arena with water monsters.
	{{10, 19}, {90, 120},	20,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR | BP_REQUIRE_CHALLENGE_PARENT),	{
		{0,			FLOOR_FLOODABLE,LIQUID,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{DF_SPREADABLE_WATER_POOL,0,0,          {2, 4},		1,			0,			-1,			0,				0,				4,				HORDE_MACHINE_WATER_MONSTER,0,(MF_GENERATE_HORDE)},
		{0,			WALL_LEVER,	DUNGEON,		{1,1},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_FAR_FROM_ORIGIN)},
		{0,			FLAMETHROWER_DORMANT,DUNGEON,{2,4},		2,			0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,			ARENA_CAGE_CLOSED,DUNGEON,{1,1},		1,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)},
		{0,			ARENA_CAGE_CLOSED,DUNGEON,{2,4},		2,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING)}}},
	// Sticky bundles - key is in bundle. Bundles must be burned open which awakes spiders hidden in webs. Some bundles contain hostile monsters.
	{{10, AMULET_LEVEL}, {55,100},	5,		7,			(BP_ADOPT_ITEM | BP_REQUIRE_CHALLENGE_PARENT | BP_PURGE_LIQUIDS),	{
		{0,			SPIDERWEB,SURFACE,			{25,35},	1,			0,			-1,			0,				0,				0,				0,			0,			(MF_PERMIT_BLOCKING)},
		{0,			STICKY_BUNDLE_DORMANT,SURFACE,{1,1},	1,			0,			-1,			0,				0,				2,				0,		0,				(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			STICKY_BUNDLE_EMPTY,SURFACE,{1,3},		0,			0,			-1,			0,				0,				2,				0,		0,				(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			STICKY_BUNDLE_DORMANT,SURFACE,{1,3},	1,			0,			-1,			0,				0,				2,				(HORDE_MACHINE_BUNDLE), 0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_TREAT_AS_BLOCKING)},
		{0,			SPIDERWEB_DORMANT,SURFACE,	{3, 5},		1,			0,			-1,			0,				MK_SPIDER,		3,				0,			0,			(MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			0,			0,				{1,1},		1,			POTION,		POTION_WINDS,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY)},
		{0,			FLAMETHROWER_HIDDEN,DUNGEON,{1,1},		1,			0,			0,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
	// Crystal Prism - key is with monster in a crystal prism. Shatter the prism using scroll of shattering or open with lever and fight the monster.
	{{10, 19},		{35,70},	15,		4,			(BP_ADOPT_ITEM | BP_REQUIRE_CHALLENGE_PARENT),	{
		{DF_SURROUND_CRYSTAL_PRISM,ALTAR_INERT,DUNGEON,	{1,1},1,		0,			-1,			0,				0,				3,				HORDE_MACHINE_PRISM,0,	(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_GENERATE_HORDE)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			0,			0,				{1,1},		1,			SCROLL,		SCROLL_SHATTERING,0,		0,				0,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,			0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE)}}},
	// Capacitor chamber - key is held by monster hidden in a chamber. Charge the capacitors with lightning, either through a staff or by routing sparks from a charged capacitor, and fight the monster.
	{{10, AMULET_LEVEL}, {35,80},	15,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR | BP_REQUIRE_CHALLENGE_PARENT),	{
		{0,			CHAMBER_CLOSED,	DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				3,				HORDE_MACHINE_CHAMBER,0,(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_MONSTER_TAKE_ITEM | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)},
		{0,			CAPACITOR_CHARGED,DUNGEON,	{1,1},		1,			0,			0,			0,				MK_SPARK_TURRET,4,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT), (MF2_SET_AS_TARGET)},
		{0,			CAPACITOR_UNCHARGED,DUNGEON,{2,5},		2,			0,			0,			0,				MK_SPARK_TURRET,3,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT), (MF2_IN_VIEW_OF_TARGET | MF2_BRIDGE_TO_TARGET)},
		{0,			0,			0,				{2,2},		2,			POTION,		POTION_WATER,0,				0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			0,			0,				{1,1},		1,			WEAPON,		GRAPPLING_DART,0,			0,				1,				0,			0,			(MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
		{0,			TORCH_WALL,	DUNGEON,		{1,4},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)}}},
	// Iron Golem Boss - key is held by an iron golem in a secret hallway filled with glyphs and guardians and surrounded by statues.
	{{20, AMULET_LEVEL}, {75, 120},	10,		6,			(BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_REQUIRE_CHALLENGE_PARENT),	{
		{0,			SECRET_DOOR,DUNGEON,		{1,1},		1,			0,			0,			0,				0,				3,				0,			0,			(MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
		{0,			0,          0,              {3,6},		3,			0,			-1,			0,				MK_GUARDIAN,	2,				0,			0,			(MF_GENERATE_MONSTER | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_PERMIT_BLOCKING)},
		{0,         MACHINE_GLYPH,DUNGEON,      {10,15},   10,          0,          -1,         0,              0,              1,              0,          0,			(MF_NOT_IN_HALLWAY)},
		{0,			TORCH_WALL,	DUNGEON,		{3,5},		0,			0,			0,			0,				0,				1,				0,			0,			(MF_BUILD_IN_WALLS)},
		{0,			STATUE_INERT,DUNGEON,		{5,10},		0,			0,			-1,			0,				0,				2,				0,          0,			(MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS)},
		{DF_RUBBLE,	0,			0,				{1,1},		1,			0,			-1,			0,				MK_IRON_GOLEM,	1,				0,			0,			(MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_MONSTER_TAKE_ITEM | MF_GENERATE_MONSTER | MF_MONSTER_SLEEPING)}}},
	// Give up a lumenstone for a challenge reward.
	{{20, AMULET_LEVEL},{10, 30},	5,		3,			(BP_REWARD | BP_ADOPT_ITEM | BP_REQUIRE_CHALLENGE_PARENT),	{
		{DF_GRASS,	FOLIAGE,	SURFACE,		{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,			ALTAR_GEM_SACRIFICE,DUNGEON,	{1,1},	0,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			0,			0,			0,				0,				2,				0,			0,			(MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)}}},

	// -- STARTER MACHINES --

	// Starter kit -- attack staff, sacrifice staff to get chain mail, mid-tier weapon, scroll of protect weapon and armor
	{{1, 1},	{60, 90},			0,		13,			(BP_REWARD),	{
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(ARMOR),	CHAIN_MAIL,	0,				0,				2,				0,			0,	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(WEAPON),	SWORD,		0,				0,				2,				0,			0,	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(WEAPON),	SPEAR,		0,				0,				2,				0,			0,	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(WEAPON),	AXE,		0,				0,				2,				0,			0,	(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(SCROLL),	SCROLL_PROTECT_ARMOR,0,		0,				2,				0,			(ITEM_KIND_AUTO_ID),(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(SCROLL),	SCROLL_PROTECT_WEAPON,0,	0,				2,				0,			(ITEM_KIND_AUTO_ID),(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			(STAFF),	STAFF_FORCE,0,				0,				2,				0,			(ITEM_PLAYER_AVOIDS), (MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_GENERATE_ITEM)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			(STAFF),	STAFF_LIGHTNING,0,			0,				2,				0,			(ITEM_PLAYER_AVOIDS), (MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_GENERATE_ITEM)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			(STAFF),	STAFF_FIRE,0,				0,				2,				0,			(ITEM_PLAYER_AVOIDS), (MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_GENERATE_ITEM)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			(STAFF),	STAFF_POISON,0,				0,				2,				0,			(ITEM_PLAYER_AVOIDS), (MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_GENERATE_ITEM)},
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON, {1,1},	1,			(STAFF),	STAFF_CONJURATION,0,		0,				2,				0,			(ITEM_PLAYER_AVOIDS), (MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_GENERATE_ITEM)},
		{0,			STATUE_INERT,DUNGEON,		{2,4},		0,			0,			-1,			0,				0,				2,				0,          0,          (MF_BUILD_IN_WALLS | MF_TREAT_AS_BLOCKING)},
		{DF_GRASS,	FOLIAGE,	SURFACE,		{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0}}},

	// Starter talisman pack -- like talisman library, but appears at the start of the game, and doesn't require a challenge to get
	{{1, 1},{30, 50},	1,		3,			(BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_REWARD),	{
		{0,			CARPET,		DUNGEON,		{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			0,          0,              {1,1},		1,			0,          0,          0,				0,				2,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
		{0,			ALTAR_CAGE_OPEN,DUNGEON,	{3,3},		3,			(TALISMAN),	-1,			0,				0,				2,				0,			(ITEM_IS_KEY | ITEM_NAMED | ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING), (MF2_REQUIRE_NOT_NEGATIVE)}}},

	// -- FLAVOR MACHINES --

	// Gambling altar - test your luck against the 'gods'
	{{5, 19},{35, 60},	0,		12,				0,	{
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_REVEAL,DUNGEON,{1,1},1,		TOME,		TOME_IDENTIFY,0,			0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_REVEAL,DUNGEON,{1,1},1,		ELIXIR,		ELIXIR_DETECT_MAGIC,0,		0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_REWARD,DUNGEON,{1,1},1,		SCROLL,		SCROLL_ENCHANTING,0,		0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_REWARD,DUNGEON,{1,1},1,		SCROLL,		SCROLL_DUPLICATION,0,		0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_PUNISH,DUNGEON,{1,1},1,		SCROLL,		SCROLL_NEGATION,0,			0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_PUNISH,DUNGEON,{1,1},1,		POTION,		POTION_DESCENT,0,			0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{DF_SURROUND_OBSIDIAN,ALTAR_CAGE_PUNISH,DUNGEON,{1,1},1,		POTION,		POTION_INCINERATION,0,		0,				3,				0,			0,			(MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN | MF_GENERATE_ITEM)},
		{0,			ALTAR_ITEM_GAMBLE,DUNGEON,	{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)},
		{0,			GRAFFITI_1, SURFACE,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE_2), (MF2_ADJACENT_TO_ORIGIN | MF_NOT_IN_HALLWAY)},
		{0,			GRAFFITI_2, SURFACE,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE_2), (MF2_ADJACENT_TO_ORIGIN | MF_NOT_IN_HALLWAY)},
		{0,			GRAFFITI_3, SURFACE,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE_2), (MF2_ADJACENT_TO_ORIGIN | MF_NOT_IN_HALLWAY)},
		{0,			GRAFFITI_4, SURFACE,		{1,1},		1,			0,			-1,			0,				0,				2,				0,			0,			(MF_ALTERNATIVE_2), (MF2_ADJACENT_TO_ORIGIN | MF_NOT_IN_HALLWAY)}}},
	// Potion experiment - combine potions
	{{3, 15},	{30, 40},	0,		3,			(BP_TREAT_AS_BLOCKING),	{
		{0,			ALEMBIC_POTION_BURN_UNUSED, DUNGEON, {1,1},	1,		0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALEMBIC_POTION_UNUSED, DUNGEON, {1,1},	1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			CAPACITOR_CHARGED, DUNGEON,	{2,3},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
	// Scroll experiment - combine scrolls
	{{3, 15},	{30, 40},	0,		3,			(BP_TREAT_AS_BLOCKING),	{
		{0,			PRESS_SCROLL_BURN_UNUSED, DUNGEON, {1,1},1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			PRESS_SCROLL_UNUSED, DUNGEON, {1,1},	1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			CAPACITOR_CHARGED, DUNGEON,	{2,3},		1,			0,			0,			0,				0,				2,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
	// Sacrifice -- Give up food or a staff or ring for scroll of duplication or enchantment or potion of strength or life or an unknown talisman. No key needed.
	{{1, 9},	{20, 30},	0,		9,			0,	{
		{0,			ALTAR_STAFF_SACRIFICE,DUNGEON,	{1,1},	1,			0,			0,			0,				0,				3,				0,			0,			(MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_FOOD_SACRIFICE,DUNGEON,	{1,1},	1,			0,			0,			0,				0,				3,				0,			0,			(MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_RING_SACRIFICE,DUNGEON,	{1,1},	1,			0,			0,			0,				0,				3,				0,			0,			(MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(TALISMAN),	-1,			0,				0,				3,				0,			0,			(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY), (MF2_REQUIRE_NOT_NEGATIVE)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(SCROLL),	SCROLL_ENCHANTING,0,		0,				3,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(SCROLL),	SCROLL_DUPLICATION,0,		0,				3,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(POTION),	POTION_STRENGTH,0,			0,				3,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{0,			ALTAR_SACRIFICE_REWARD,DUNGEON,	{1,1},	1,			(POTION),	POTION_LIFE,0,				0,				3,				0,			(ITEM_KIND_AUTO_ID),	(MF_GENERATE_ITEM | MF_ALTERNATIVE_2 | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
		{DF_GRASS,	FOLIAGE,	SURFACE,			{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0}}},
	// Excess capacity -- Avoid hitting capacitors with lightning
	{{11, AMULET_LEVEL}, {40, 70},	0,		1,			(0),	{
		{0,			CAPACITOR_UNCHARGED,DUNGEON,	{2,5},	2,			0,			0,			0,				MK_SPARK_TURRET,3,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT), (MF2_SET_AS_TARGET | MF2_IN_VIEW_OF_TARGET | MF2_BRIDGE_TO_TARGET)}}},
	// Witch hazel -- witch hazel plants, and surrounding hay
	{{5,100},	{5, 5},     0,          1,			(BP_TREAT_AS_BLOCKING), {
		{DF_HAY,	WITCH_HAZEL_FLOWER,	SURFACE,	{1, 2},	1,				0,			-1,			0,				0,				2,				0,			0,			(MF_NOT_IN_HALLWAY)}}},
	// Mandrake roots -- mandrake sacs, mandrake roots and luminescent fungus
	{{7,100},	{10, 10},     0,        2,			(BP_TREAT_AS_BLOCKING), {
		{DF_LUMINESCENT_FUNGUS,	MANDRAKE_SAC_MACHINE,	SURFACE,{1, 1},	1,	0,		-1,			0,				0,				1,				(HORDE_MACHINE_MANDRAKE),0,	(MF_BUILD_AT_ORIGIN | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_MONSTER_LIFESPAN)},
		{DF_MANDRAKE_ROOTS_BUILD,	0,	SURFACE,{1, 1},	1,				0,			-1,			0,				0,				1,				0,			0,			(MF_BUILD_AT_ORIGIN)}}},
	// Stinkfruit bushes -- stinkfruit stalk, some pods, blocking traffic
	{{4,100},	{20, 20},     0,		2,			(BP_REQUIRE_BLOCKING), {
		{0,			DEAD_GRASS,			SURFACE,	{1, 1},     1,		0,			-1,			0,				0,				0,				0,			0,          (MF_EVERYWHERE)},
		{DF_STINKFRUIT_PODS_GROW_INITIAL,STINKFRUIT_STALK,	SURFACE,{2, 3},	2,0,	-1,			0,				0,				1,				0,			0,			(MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
	// Crimson cap mushrooms -- marked so that it is visible from anywhere on the level
	{{7,100},	{30, 30},     0,		1,			(BP_REQUIRE_BLOCKING), {
		{DF_LUMINESCENT_FUNGUS,		0,	0,			{3, 5},		3,		0,			-1,			0,				0,				2,				(HORDE_MACHINE_CRIMSON_CAP),0,	(MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE)}}},

	// Bloodwort -- bloodwort stalk, some pods, and surrounding grass
	{{1,100},	{5, 5},     0,			  2,			(BP_TREAT_AS_BLOCKING), {
		{DF_GRASS,	BLOODFLOWER_STALK, SURFACE,	{1, 1},		1,			0,			-1,			0,				0,				0,				0,			0,			(MF_BUILD_AT_ORIGIN | MF_NOT_IN_HALLWAY)},
		{DF_BLOODFLOWER_PODS_GROW_INITIAL,0, 0, {1, 1},     1,			0,			-1,			0,				0,				1,				0,			0,          (MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING)}}},
	// Idyll -- ponds and some grass and forest
	{{1,100},			{80, 120},	0,		2,			BP_NO_INTERIOR_FLAG, {
		{DF_GRASS,	FOLIAGE,	SURFACE,		{3, 4},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_WATER_POOL,	0,		0,				{2, 3},		2,			0,			-1,			0,				0,				5,				0,			0,			(MF_NOT_IN_HALLWAY)}}},
	// Swamp -- mud, grass and some shallow water
	{{1,100},			{50, 65},	0,		2,			BP_NO_INTERIOR_FLAG, {
		{DF_SWAMP,	0,			0,				{6, 8},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{DF_WATER_POOL,	0,		0,				{0, 1},		0,			0,			-1,			0,				0,				3,				0,			0,			(MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
	// Camp -- hay, junk, urine, vomit
	{{1,100},			{40, 50},	0,		4,			BP_NO_INTERIOR_FLAG, {
		{DF_HAY,	0,			0,				{1, 3},		1,			0,			-1,			0,				0,				1,				0,			0,			(MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
		{DF_JUNK,	0,			0,				{1, 2},		1,			0,			-1,			0,				0,				3,				0,			0,			(MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
		{DF_URINE,	0,			0,				{3, 5},		1,			0,			-1,			0,				0,				1,				0,			0,			MF_IN_VIEW_OF_ORIGIN},
		{DF_VOMIT,	0,			0,				{0, 2},		0,			0,			-1,			0,				0,				1,				0,			0,			MF_IN_VIEW_OF_ORIGIN}}},
	// Remnant -- carpet surrounded by ash and with some statues
	{{1,100},			{80, 120},	0,		2,			BP_NO_INTERIOR_FLAG, {
		{DF_REMNANT, 0,			0,				{6, 8},		3,			0,			-1,			0,				0,				1,				0,			0,			0},
		{0,				STATUE_INERT,DUNGEON,	{3, 5},		2,			0,			-1,			0,				0,				1,				0,			0,			(MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
	// Dismal -- blood, bones, charcoal, some rubble
	{{1,100},			{60, 70},	0,		3,			BP_NO_INTERIOR_FLAG, {
		{DF_AMBIENT_BLOOD, 0,	0,				{5,10},		3,			0,			-1,			0,				0,				1,				0,			0,			MF_NOT_IN_HALLWAY},
		{DF_ASH,	0,			0,				{4, 8},		2,			0,			-1,			0,				0,				1,				0,			0,			MF_NOT_IN_HALLWAY},
		{DF_BONES,	0,			0,				{3, 5},		2,			0,			-1,			0,				0,				1,				0,			0,			MF_NOT_IN_HALLWAY}}},
	// Chasm catwalk -- narrow bridge over a chasm, possibly under fire from a turret or two
	{{1,99},			{40, 80},	0,		4,			(BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
		{DF_CHASM_HOLE,	0,		0,				{80, 80},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{DF_CATWALK_BRIDGE,0,	0,				{0,0},		0,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)},
		{0,			MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},	0,			0,			0,			0,				0,				1,				0,			0,			(MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
		{0,			TURRET_DORMANT,DUNGEON,		{1, 2},		1,			0,			-1,			0,				0,				2,				HORDE_MACHINE_TURRET,0,	(MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
	// Lake walk -- narrow bridge of shallow water through a lake, possibly under fire from a turret or two
	{{1,100},			{40, 80},	0,		3,			(BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
		{DF_LAKE_CELL,	0,		0,				{80, 80},	1,			0,			-1,			0,				0,				1,				0,			0,			(MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
		{0,			MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},	0,			0,			0,			0,				0,				1,				0,			0,			(MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
		{0,			TURRET_DORMANT,DUNGEON,		{1, 2},		1,			0,			-1,			0,				0,				2,				HORDE_MACHINE_TURRET,0,	(MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    // Paralysis trap -- hidden pressure plate with a few vents nearby.
    {{1,100},			{35, 40},	0,		2,			(BP_NO_INTERIOR_FLAG), {
		{0,         GAS_TRAP_PARALYSIS_HIDDEN, DUNGEON, {1,2},1,0,		0,			0,			0,				3,				0,			0,			(MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY)},
		{0,			MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{3, 4},2,		0,			0,			0,				0,				3,				0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
	// Statue comes alive -- innocent-looking statue that bursts to reveal a monster when the player approaches
	{{1,100},			{5, 5},		0,		3,			(BP_NO_INTERIOR_FLAG),	{
		{0,			STATUE_DORMANT,DUNGEON,		{1, 1},		1,			0,			-1,			0,				0,				1,				HORDE_MACHINE_STATUE,0,	(MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
		{0,			STATUE_DORMANT,DUNGEON,		{1, 1},		1,			0,			-1,			0,				0,				1,				HORDE_MACHINE_STATUE,0,	(MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_ALTERNATIVE | MF_NOT_ON_LEVEL_PERIMETER)},
		{0,			MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},	2,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)}}},
	// Worms in the walls -- step on trigger region to cause underworms to burst out of the walls
	{{1,100},			{7, 7},		0,		2,			(BP_NO_INTERIOR_FLAG),	{
		{0,			WALL_MONSTER_DORMANT,DUNGEON, {1, 3},	1,			0,			-1,			0,				MK_UNDERWORM,	1,				0,			0,			(MF_GENERATE_MONSTER | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)},
		{0,			MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},	2,			0,			-1,			0,				0,				0,				0,			0,			(MF_EVERYWHERE)}}},	
	// Sentinels
	{{1,100},			{40, 40},	0,		2,			(BP_NO_INTERIOR_FLAG), {
		{0,			STATUE_INERT,DUNGEON,		{3, 3},		3,			0,			-1,			0,				MK_SENTINEL,	2,				0,			0,			(MF_GENERATE_MONSTER | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN)},
		{DF_ASH,	0,			0,				{2, 3},		0,			0,			-1,			0,				0,				0,				0,			0,			0}}},
};

#pragma mark Monster definitions

// Defines all creatures, which include monsters and the player:
creatureType monsterCatalog[NUMBER_MONSTER_KINDS] = {
	//	name			ch		color			HP		def		acc		damage			reg	sight	scent	move	attack	blood			light	DFChance DFType		behaviorF, abilityF slayID
	{0,	"you",	PLAYER_CHAR,	&playerInLightColor,30,	0,		100,	{1, 2, 1},		20,	DCOLS,	30,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE)},

	{0, "rat",			'r',	&gray,			6,		0,		80,		{1, 3, 1},		20,	20,		30,		100,	100,	DF_RED_BLOOD,	0,		1,		DF_URINE},
	{0, "kobold",		'k',	&goblinColor,	7,		0,		80,		{1, 4, 1},		20,	30,		30,		100,	100,	DF_RED_BLOOD,	0,		0,		0},
	{0,	"jackal",		'j',	&jackalColor,	8,		0,		70,		{2, 4, 1},		20,	50,		50,		50,		100,	DF_RED_BLOOD,	0,		1,		DF_URINE},
	{0,	"eel",			'e',	&eelColor,		18,		27,		100,	{3, 7, 2},		5,	DCOLS,	20,		50,		100,	0,              0,		0,		0,
		(MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_WILL_NOT_USE_STAIRS)},
	{0,	"monkey",		'm',	&ogreColor,		12,		17,		100,	{1, 3, 1},		20,	DCOLS,	100,	100,	100,	DF_RED_BLOOD,	0,		1,		DF_URINE,
		(0), (MA_HIT_STEAL_FLEE)},
	{0, "bloat",		'b',	&poisonGasColor,4,		0,		100,	{0, 0, 0},		5,	DCOLS,	100,	100,	100,	DF_PURPLE_BLOOD,0,		0,		DF_BLOAT_DEATH,
		(MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
	{0, "pit bloat",	'b',	&blue,          4,		0,		100,	{0, 0, 0},		5,	DCOLS,	100,	100,	100,	DF_PURPLE_BLOOD,0,		0,		DF_HOLE_POTION,
		(MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH), MK_BLOAT},
	{0, "goblin",		'g',	&goblinColor,	15,		10,		70,		{2, 5, 1},		20,	30,		20,		100,	100,	DF_RED_BLOOD,	0,		0,		0},
	{0, "goblin conjurer",'g',	&goblinConjurerColor, 10,10,	70,		{2, 4, 1},		20,	30,		20,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON), MK_GOBLIN},
	{0, "goblin mystic",'g',	&goblinMysticColor, 10,	10,		70,		{2, 4, 1},		20,	30,		20,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_CAST_PROTECTION), MK_GOBLIN},
	{0, "goblin totem",	TOTEM_CHAR,	&orange,	30,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,	300,	DF_RUBBLE_BLOOD,IMP_LIGHT,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_INTRINSIC_LIGHT | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS), (MA_CAST_HASTE | MA_CAST_SPARK)},
	{0, "pink jelly",	'J',	&pinkJellyColor,50,		0,		85,		{1, 3, 1},		0,	20,		20,		100,	100,	DF_PURPLE_BLOOD,0,		0,		0,
		(MONST_NEVER_SLEEPS), (MA_CLONE_SELF_ON_DEFEND), MK_JELLY},
	{0, "toad",			't',	&toadColor,		18,		0,		90,		{1, 4, 1},		10,	15,		15,		100,	100,	DF_GREEN_BLOOD,	0,		0,		0,
		(0), (MA_HIT_HALLUCINATE)},
	{0, "vampire bat",	'v',	&gray,			18,		25,		100,	{2, 6, 1},		20,	DCOLS,	50,		50,		100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_FLIES | MONST_FLITS), (MA_TRANSFERENCE), MK_VAMPIRE},
	{0, "arrow turret", TURRET_CHAR,&black,		30,		0,		90,		{2, 6, 1},		0,	DCOLS,	50,		100,	250,	0,              0,		0,		0,
		(MONST_TURRET), (MA_ATTACKS_FROM_DISTANCE)},
	{0, "acid mound",	'a',	&acidBackColor,	15,		10,		70,		{1, 3, 1},		5,	15,		15,		100,	100,	DF_ACID_BLOOD,	0,		0,		0,
		(MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR)},
	{0, "centipede",	'c',	&centipedeColor,20,		20,		80,		{4, 12, 1},		20,	20,		50,		100,	100,	DF_GREEN_BLOOD,	0,		0,		0,
		(0), (MA_CAUSES_WEAKNESS)},
	{0,	"ogre",			'O',	&ogreColor,		35,		60,		125,	{9, 13, 2},		20,	30,		30,		100,	200,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE | MONST_SHIELD_BLOCKS)},
	{0,	"bog monster",	'B',	&krakenColor,	55,		60,		5000,	{3, 4, 1},		3,	30,		30,		200,	100,	0,              0,		0,		0,
		(MONST_RESTRICTED_TO_LIQUID | MONST_SUBMERGES | MONST_FLITS | MONST_FLEES_NEAR_DEATH | MONST_WILL_NOT_USE_STAIRS), (MA_SEIZES)},
	{0, "ogre totem",	TOTEM_CHAR,	&green,		70,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,	400,	DF_RUBBLE_BLOOD,LICH_LIGHT,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_INTRINSIC_LIGHT | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS), (MA_CAST_HEAL | MA_CAST_SLOW)},
	{0, "spider",		's',	&white,			20,		70,		90,		{6, 11, 2},		20,	50,		20,		100,	100,	DF_GREEN_BLOOD,	0,		0,		0,
		(MONST_IMMUNE_TO_WEBS | MONST_CAST_SPELLS_SLOWLY), (MA_SHOOTS_WEBS | MA_POISONS)},
	{0, "spark turret", TURRET_CHAR, &lightningColor,80,0,		100,	{0, 0, 0},		0,	DCOLS,	50,		100,	150,	0,              SPARK_TURRET_LIGHT,	0,	0,
		(MONST_TURRET | MONST_INTRINSIC_LIGHT), (MA_CAST_SPARK)},
	{0,	"will-o-the-wisp",'w',	&wispLightColor,10,		90,     100,	{5,	8, 2},		5,	90,		15,		100,	100,	DF_ASH_BLOOD,	WISP_LIGHT,	0,	0,
		(MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_INTRINSIC_LIGHT | MONST_DIES_IF_NEGATED)},
	{0, "harpy",		'h',	&gray,			20,		25,		175,	{2, 6, 1},		20,	DCOLS,	50,		50,		50,		DF_RED_BLOOD,	0,		0,		0,
		(MONST_FLIES | MONST_FEMALE), (MA_HIT_STEAL_FLEE)},
	{0, "wraith",		'W',	&wraithColor,	50,		60,		120,	{6, 13, 2},		5,	DCOLS,	100,	50,		100,	DF_GREEN_BLOOD,	0,		0,		0,
		(MONST_FLEES_NEAR_DEATH)},
	{0, "lamia",		'l',	&green,			30,		40,		110,	{12, 18, 2},	3,	DCOLS,	100,	50,		200,	DF_RED_BLOOD,	LICH_LIGHT,0,	0,
		(MONST_NEVER_SLEEPS | MONST_FEMALE | MONST_IMMUNE_TO_WATER | MONST_INTRINSIC_LIGHT), (MA_CAST_BECKONING | MA_POISONS)},
	{0, "zombie",		'Z',	&vomitColor,	80,		0,		120,	{7, 12, 1},		0,	50,		200,	100,	100,	DF_ROT_GAS_BLOOD,0,		100,	DF_ROT_GAS_PUFF, (0)},
	{0, "troll",		'T',	&trollColor,	65,		70,		125,	{10, 15, 3},		1,	DCOLS,	20,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE)},
	{0,	"ogre shaman",	'O',	&green,			45,		40,		100,	{5, 9, 1},		20,	DCOLS,	30,		100,	200,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_MALE | MONST_FEMALE), (MA_CAST_HASTE | MA_CAST_SPARK | MA_CAST_SUMMON), MK_OGRE},
	{0, "naga",			'N',	&trollColor,	75,		70,     150,	{7, 11, 4},		10,	DCOLS,	100,	100,	100,	DF_GREEN_BLOOD,	0,		100,	DF_PUDDLE,
		(MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FEMALE | MONST_FLEES_NEAR_DEATH)},
	{0, "salamander",	'S',	&salamanderColor,60,	70,     150,	{7, 13, 3},		10,	DCOLS,	100,	100,	100,	DF_ASH_BLOOD,	SALAMANDER_LIGHT, 100, DF_SALAMANDER_FLAME,
		(MONST_IMMUNE_TO_FIRE | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_INTRINSIC_LIGHT | MONST_MALE | MONST_FLEES_NEAR_DEATH)},
	{0, "explosive bloat",'b',	&orange,		10,		0,		100,	{0, 0, 0},		5,	DCOLS,	100,	100,	100,	DF_RED_BLOOD,	EMBER_LIGHT,0,	DF_BLOAT_EXPLOSION,
		(MONST_FLIES | MONST_FLITS| MONST_INTRINSIC_LIGHT), (MA_KAMIKAZE | MA_DF_ON_DEATH), MK_BLOAT},
	{0, "dar blademaster",'d',	&purple,		35,		70,     160,	{5, 9, 2},		20,	DCOLS,	100,	100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_CAST_BLINK), MK_DAR},
	{0, "dar priestess", 'd',	&darPriestessColor,20,	60,		100,	{2, 5, 1},		20,	DCOLS,	100,	100,	100,	DF_RED_BLOOD,   0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_FEMALE), (MA_CAST_HEAL | MA_CAST_SPARK | MA_CAST_HASTE | MA_CAST_NEGATION), MK_DAR},
	{0, "dar battlemage",'d',	&darMageColor,	20,		60,		100,	{1, 3, 1},		20,	DCOLS,	100,	100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_CAST_FIRE | MA_CAST_SLOW | MA_CAST_DISCORD), MK_DAR},
	{0, "acidic jelly",	'J',	&acidBackColor,	60,		0,		115,	{2, 6, 1},		0,	20,		20,		100,	100,	DF_ACID_BLOOD,	0,		0,		0,
		(MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR | MA_CLONE_SELF_ON_DEFEND), MK_JELLY},
	{0,	"centaur",		'C',	&tanColor,		35,		50,		175,	{4, 8, 2},		20,	DCOLS,	30,		50,		100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_MALE), (MA_ATTACKS_FROM_DISTANCE)},
	{0, "underworm",	'U',	&wormColor,		80,		40,		160,	{18, 22, 2},	3,	20,		20,		150,	200,	DF_WORM_BLOOD,	0,		0,		0,
		(MONST_NEVER_SLEEPS)},
	{0, "sentinel",		STATUE_CHAR, &sentinelColor, 50,0,		0,		{0, 0, 0},		0,	DCOLS,	100,	100,	175,	DF_RUBBLE_BLOOD,SENTINEL_LIGHT,0,0,
		(MONST_TURRET | MONST_INTRINSIC_LIGHT | MONST_CAST_SPELLS_SLOWLY | MONST_DIES_IF_NEGATED), (MA_CAST_HEAL | MA_CAST_SPARK)},
	{0, "acid turret", TURRET_CHAR,	&acidBackColor,35,	0,		250,	{1, 2, 1},      0,	DCOLS,	50,		100,	250,	0,              0,		0,		0,
		(MONST_TURRET), (MA_ATTACKS_FROM_DISTANCE | MA_HIT_DEGRADE_ARMOR)},
	{0, "dart turret", TURRET_CHAR,	&centipedeColor,20,	0,		140,	{1, 2, 1},      0,	DCOLS,	50,		100,	250,	0,              0,		0,		0,
		(MONST_TURRET), (MA_ATTACKS_FROM_DISTANCE | MA_CAUSES_WEAKNESS)},
	{0,	"kraken",		'K',	&krakenColor,	120,	0,		150,	{15, 20, 3},	1,	DCOLS,	20,		50,		100,	0,              0,		0,		0,
		(MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FLEES_NEAR_DEATH | MONST_WILL_NOT_USE_STAIRS), (MA_SEIZES)},
	{0,	"lich",			'L',	&white,			35,		80,     175,	{2, 6, 1},		0,	DCOLS,	100,	100,	100,	DF_ASH_BLOOD,	LICH_LIGHT,	0,	0,
		(MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_INTRINSIC_LIGHT), (MA_CAST_SUMMON | MA_CAST_FIRE)},
	{0, "phylactery",	GEM_CHAR,&lichLightColor,30,	0,		0,		{0, 0, 0},		0,	DCOLS,	50,		100,	150,	DF_RUBBLE_BLOOD,LICH_LIGHT,	0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_INTRINSIC_LIGHT | MONST_DIES_IF_NEGATED), (MA_CAST_SUMMON | MA_ENTER_SUMMONS), MK_LICH},
	{0, "pixie",		'p',	&pixieColor,	10,		90,     100,	{1, 3, 1},		20,	100,	100,	50,		100,	DF_GREEN_BLOOD,	PIXIE_LIGHT, 0,	0,
		(MONST_MAINTAINS_DISTANCE | MONST_INTRINSIC_LIGHT | MONST_FLIES | MONST_FLITS | MONST_MALE | MONST_FEMALE), (MA_CAST_SPARK | MA_CAST_SLOW | MA_CAST_NEGATION | MA_CAST_DISCORD)},
	{0,	"phantom",		'P',	&ectoplasmColor,35,		70,     160,	{12, 18, 4},		0,	30,		30,		50,		200,	DF_ECTOPLASM_BLOOD,	0,	2,		DF_ECTOPLASM_DROPLET,
		(MONST_INVISIBLE | MONST_FLITS | MONST_FLIES | MONST_IMMUNE_TO_WEBS)},
	{0, "flame turret", TURRET_CHAR, &lavaForeColor,40,	0,		150,	{1, 2, 1},		0,	DCOLS,	50,		100,	250,	0,              LAVA_LIGHT,	0,	0,
		(MONST_TURRET | MONST_INTRINSIC_LIGHT), (MA_CAST_FIRE)},
	{0, "imp",			'i',	&pink,			35,		90,     225,	{4, 9, 2},		10,	10,		15,		100,	100,	DF_GREEN_BLOOD,	IMP_LIGHT,	0,	0,
		(MONST_INTRINSIC_LIGHT | MONST_FLEES_NEAR_DEATH), (MA_HIT_STEAL_FLEE | MA_CAST_BLINK)},
	{0,	"fury",			'f',	&darkRed,		19,		90,     200,	{6, 11, 4},		20,	40,		30,		50,		100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_NEVER_SLEEPS | MONST_FLIES)},
	{0, "revenant",		'R',	&ectoplasmColor,30,		0,		200,	{15, 20, 5},	0,	DCOLS,	20,		100,	100,	DF_ECTOPLASM_BLOOD,	0,	0,		0,
		(MONST_IMMUNE_TO_WEAPONS)},
	{0, "tentacle horror",'H',	&centipedeColor,120,	95,     225,	{25, 35, 3},	1,	DCOLS,	50,		100,	100,	DF_PURPLE_BLOOD,0,		0,		0,
		(0), (MA_ATTACKS_PENETRATE)},
	{0, "golem",		'G',	&gray,			400,	70,     225,	{4, 8, 1},		0,	DCOLS,	200,	100,	100,	DF_RUBBLE_BLOOD,0,		0,		0,
		(MONST_REFLECT_4 | MONST_DIES_IF_NEGATED)},
	{0, "dragon",		'D',	&dragonColor,	150,	90,     250,	{25, 50, 4},	20,	DCOLS,	120,	50,		200,	DF_GREEN_BLOOD,	0,		0,		0,
		(MONST_IMMUNE_TO_FIRE | MONST_CARRY_ITEM_100), (MA_BREATHES_FIRE | MA_ATTACKS_ALL_ADJACENT)},
	{0, "dragon",		'D',	&dragonColor,	150,	90,     250,	{25, 50, 4},	20,	DCOLS,	120,	50,		200,	DF_GREEN_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_IMMUNE_TO_FIRE | MONST_CARRY_ITEM_100), (MA_BREATHES_FIRE | MA_ATTACKS_ALL_ADJACENT)},
	{0, "revenant totem",TOTEM_CHAR, &yellow,	110,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,300,	DF_RUBBLE_BLOOD,WISP_LIGHT,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_INTRINSIC_LIGHT | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS), (MA_CAST_INVISIBILITY | MA_CAST_PROTECTION)},
	{0, "dar totem",TOTEM_CHAR, &purple,	150,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,300,		DF_RUBBLE_BLOOD,UNICORN_LIGHT,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_INTRINSIC_LIGHT | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS), (MA_CAST_BECKONING | MA_CAST_SUMMON)},
	{0, "dragon totem",TOTEM_CHAR, &red,	150,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,300,		DF_RUBBLE_BLOOD,LAVA_LIGHT,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_INTRINSIC_LIGHT | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS), (MA_CAST_HEAL | MA_CAST_HASTE)},

	// bosses
	{0, "goblin warlord",'g',	&blue,			30,		17,		100,	{3, 6, 1},		20,	30,		20,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON), MK_GOBLIN},
	{0,	"black jelly",	'J',	&black,			120,	0,		130,	{3, 8, 1},		0,	20,		20,		100,	100,	DF_PURPLE_BLOOD,0,		0,		0,
		(0), (MA_CLONE_SELF_ON_DEFEND)},
	{0, "vampire",		'V',	&white,			75,		60,     120,	{4, 15, 2},		6,	DCOLS,	100,	50,		100,	DF_RED_BLOOD,	0,		0,		DF_BLOOD_EXPLOSION,
		(MONST_FLEES_NEAR_DEATH | MONST_MALE), (MA_CAST_BLINK | MA_CAST_DISCORD | MA_TRANSFERENCE | MA_DF_ON_DEATH | MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
	{0, "flamedancer",	'F',	&white,			65,		80,     120,	{3, 8, 2},		0,	DCOLS,	100,	100,	100,	DF_EMBER_BLOOD,	FLAMEDANCER_LIGHT,100,DF_FLAMEDANCER_CORONA,
		(MONST_MAINTAINS_DISTANCE | MONST_IMMUNE_TO_FIRE | MONST_FIERY | MONST_INTRINSIC_LIGHT), (MA_CAST_FIRE)},
	{0,	"minotaur",		'M',	&ogreColor,		110,	60,		250,	{9, 13, 2},		20,	30,		30,		100,	200,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE), (MA_HIT_KNOCKBACK)},

	// challenge bosses
	{0, "iron golem",	'G',	&darkGray,		400,	70,		225,	{4, 8, 1},		0,	DCOLS,	200,	100,	100,	DF_RUBBLE_BLOOD,0,		0,		0,
		(MONST_REFLECT_4 | MONST_CAST_SPELLS_SLOWLY), (MA_CAST_SENTRY | MA_CALL_GUARDIAN), MK_GOLEM},

	// special effect monsters
	{0, "spectral blade",WEAPON_CHAR, &spectralBladeColor,1, 0,	150,	{1, 1, 1},		0,	50,		50,		50,		100,	0,              SPECTRAL_BLADE_LIGHT,0,0,
		(MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_INTRINSIC_LIGHT | MONST_DOES_NOT_TRACK_LEADER | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS)},
	{0, "spectral sword",WEAPON_CHAR, &spectralImageColor, 1,0,	150,	{1, 1, 1},		0,	50,		50,		50,		100,	0,              SPECTRAL_IMAGE_LIGHT,0,0,
		(MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_INTRINSIC_LIGHT | MONST_DOES_NOT_TRACK_LEADER | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS)},
	{0, "stone guardian",STATUE_CHAR, &white,    1000,  0,		150,	{5, 15, 2},		0,	50,		100,	100,	100,	DF_RUBBLE,      0,      100,      DF_GUARDIAN_STEP,
		(MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_REFLECT_4 | MONST_ALWAYS_USE_ABILITY | MONST_GETS_TURN_ON_ACTIVATION)},
	{0, "winged guardian",STATUE_CHAR, &lightBlue,1000, 0,		150,	{5, 15, 2},		0,	50,		100,	100,	100,	DF_RUBBLE,      0,      100,      DF_SILENT_GLYPH_GLOW,
		(MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_REFLECT_4 | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (MA_CAST_BLINK)},
	{0, "eldritch totem",TOTEM_CHAR, &glyphColor,80,    0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,	100,	DF_RUBBLE_BLOOD,0,0,	0,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (MA_CAST_SUMMON)},
	{0, "mirrored totem",TOTEM_CHAR, &beckonColor,80,	0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,	100,	DF_RUBBLE_BLOOD,0,      100,	DF_MIRROR_TOTEM_STEP,
		(MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY | MONST_REFLECT_4 | MONST_IMMUNE_TO_WEAPONS | MONST_IMMUNE_TO_FIRE), (MA_CAST_BECKONING)},
	{0,	"mandrake",		PLAYER_CHAR, &humanBloodColor,	30,		0,		100,	{4, 12, 2},		0,	DCOLS,	30,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MAINTAINS_DISTANCE | MONST_MALE | MONST_FEMALE), (MA_CAST_SLOW | MA_CAST_DISCORD | MA_ATTACKS_ALL_ADJACENT)},
	{0,	"dead man's ear",PLAYER_CHAR, &grayFungusColor,	150, 0,	0,		{0, 0, 0},		0,	DCOLS,	200,	100,	200,	0,				0,		0,		0,
		(MONST_NOT_LISTED_IN_SIDEBAR | MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_WILL_NOT_USE_STAIRS)},
	{0,	"crimson cap",	FOLIAGE_CHAR, &markedColor,	15,		0,		0,		{0, 0, 0},		0,	DCOLS,	200,	100,	200,	0,				0,		0,		0,
		(MONST_NOT_LISTED_IN_SIDEBAR | MONST_IMMOBILE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_REFLECT_4)},

	// legendary allies
	{0,	"unicorn",		UNICORN_CHAR, &white,   40,		60,		175,	{2, 10, 2},		20,	DCOLS,	30,		50,		100,	DF_RED_BLOOD,	UNICORN_LIGHT,1,DF_UNICORN_POOP,
		(MONST_MAINTAINS_DISTANCE | MONST_INTRINSIC_LIGHT | MONST_MALE | MONST_FEMALE), (MA_CAST_HEAL | MA_CAST_PROTECTION)},
	{0,	"ifrit",		'I',	&ifritColor,	40,		75,     175,	{5, 13, 2},		1,	DCOLS,	30,		50,		100,	DF_ASH_BLOOD,	IFRIT_LIGHT,0,	0,
		(MONST_IMMUNE_TO_FIRE | MONST_INTRINSIC_LIGHT | MONST_FLIES | MONST_MALE), (MA_CAST_DISCORD)},
	{0,	"phoenix",		'P',	&phoenixColor,	30,		70,     175,	{4, 10, 2},		0,	DCOLS,	30,		50,		100,	DF_ASH_BLOOD,	PHOENIX_LIGHT,0,0,
		(MONST_IMMUNE_TO_FIRE| MONST_FLIES | MONST_INTRINSIC_LIGHT)},
	{0, "phoenix egg",	GEM_CHAR,&phoenixColor,	100,	0,		0,		{0, 0, 0},		0,	DCOLS,	50,		100,	150,	DF_ASH_BLOOD,	PHOENIX_EGG_LIGHT,	0,	0,
		(MONST_IMMUNE_TO_FIRE| MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS | MONST_INTRINSIC_LIGHT), (MA_CAST_SUMMON | MA_ENTER_SUMMONS)},

	// slay indexes - used for tidying up slay name descriptions
	{0,	"jelly",	'j',	&playerInLightColor,30,		0,		100,	{1, 2, 1},		20,	DCOLS,	30,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE)},
	{0,	"dar",		'd',	&playerInLightColor,30,		0,		100,	{1, 2, 1},		20,	DCOLS,	30,		100,	100,	DF_RED_BLOOD,	0,		0,		0,
		(MONST_MALE | MONST_FEMALE)},
};


#pragma mark Monster words

const monsterWords monsterText[NUMBER_MONSTER_KINDS] = {
	{"A naked adventurer in an unforgiving place, bereft of equipment and confused about the circumstances.",
		"studying", "Studying",
		{"hit", {0}}},
	{"The rat is a scavenger of the shallows, perpetually in search of decaying animal matter.",
		"gnawing at", "Eating",
		{"scratches", "bites", {0}}},
	{"The kobold is a lizardlike humanoid of the upper dungeon.",
		"poking at", "Examining",
		{"clubs", "bashes", {0}}},
	{"The jackal prowls the caverns for intruders to rend with $HISHER powerful jaws.",
		"tearing at", "Eating",
		{"claws", "bites", "mauls", {0}}},
	{"The eel slips silently through the subterranean lake, waiting for unsuspecting prey to set foot in $HISHER dark waters.",
		"eating", "Eating",
		{"shocks", "bites", {0}}},
	{"Mischievous trickster that $HESHE is, the monkey lives to steal shiny trinkets from passing adventurers.",
		"examining", "Examining",
		{"tweaks", "bites", "punches", {0}}},
	{"A bladder of deadly gas buoys the bloat through the air, $HISHER thin veinous membrane ready to rupture at the slightest stress.",
		"gazing at", "Gazing",
		{"bumps", {0}},
		"bursts, leaving behind an expanding cloud of caustic gas!"},
	{"This rare subspecies of bloat is filled with a peculiar vapor that, if released, will cause the floor to vanish out from underneath $HIMHER.",
		"gazing at", "Gazing",
		{"bumps", {0}},
		"bursts, causing the floor underneath $HIMHER to disappear!"},
	{"A filthy little primate, the tribalistic goblin often travels in packs and carries a makeshift stone blade.",
		"chanting over", "Chanting",
		{"slashes", "cuts", "stabs", {0}}},
	{"This goblin is covered with glowing sigils that pulse with power. $HESHE can call into existence phantom blades to attack $HISHER foes.",
		"performing a ritual on", "Performing ritual",
		{"thumps", "whacks", "wallops", {0}},
		{0},
		"gestures ominously!"},
	{"This goblin carries no weapon, and $HISHER eyes sparkle with golden light. $HESHE can invoke a powerful shielding magic to protect $HISHER escorts from harm.",
		"performing a ritual on", "Performing ritual",
		{"slaps", "punches", "kicks", {0}}},
	{"Goblins have created this makeshift totem and imbued $HIMHER with a shamanistic power.",
		"gazing at", "Gazing",
		{"hits", {0}}},
	{"This mass of poisonous pink goo slips across the ground in search of a warm meal.",
		"absorbing", "Feeding",
		{"smears", "slimes", "drenches"}},
	{"The enormous, warty toad secretes a powerful hallucinogenic slime to befuddle the senses of any creatures that come in contact with $HIMHER.",
		"eating", "Eating",
		{"slimes", "slams", {0}}},
	{"Often hunting in packs, leathery wings and keen senses guide the vampire bat unerringly to $HISHER prey.",
		"draining", "Feeding",
		{"nips", "bites", {0}}},
	{"A mechanical contraption embedded in the wall, the spring-loaded arrow turret will fire volley after volley of arrows at intruders.",
		"gazing at", "Gazing",
		{"shoots", {0}}},
	{"The acid mound squelches softly across the ground, leaving a trail of acidic goo in $HISHER path.",
		"liquefying", "Feeding",
		{"slimes", "douses", "drenches", {0}}},
	{"This monstrous centipede's incisors are imbued with a horrible venom that will slowly kill $HISHER prey.",
		"eating", "Eating",
		{"pricks", "stings", {0}}},
	{"This lumbering creature carries an enormous club that $HESHE can swing with incredible force.",
		"examining", "Studying",
		{"cudgels", "clubs", "batters", {0}}},
	{"The horrifying bog monster dwells beneath the surface of mud-filled swamps. When $HISHER prey ventures into the mud, the bog monster will ensnare the unsuspecting victim in $HISHER pale tentacles and squeeze its life away.",
		"draining", "Feeding",
		{"squeezes", "strangles", "crushes", {0}}},
	{"Ancient ogres versed in the eldritch arts have assembled this totem and imbued $HIMHER with occult power.",
		"gazing at", "Gazing",
		{"hits", {0}}},
	{"The spider's red eyes pierce the darkness in search of enemies to ensnare with $HISHER projectile webs and dissolve with deadly poison.",
		"draining", "Feeding",
		{"bites", "stings", {0}}},
	{"This contraption hums with electrical charge that $HISHER embedded crystals and magical sigils can direct at intruders in deadly arcs.",
		"gazing at", "Gazing",
		{"shocks", {0}}},
	{"An ethereal blue flame dances through the air, flickering and pulsing in time to an otherworldly rhythm.",
		"consuming", "Feeding",
		{"scorches", "burns", {0}}},
	{"A shrieking bird with a human head; and hands with long dirty nails instead of feet.",
		"eating", "Eating",
		{"grasps", "claws", "bites", {0}}},
	{"The wraith's hollow eye sockets stare hungrily at the world from $HISHER emaciated frame, and $HISHER long, bloodstained nails grope ceaselessly at the air for a fresh victim.",
		"devouring", "Feeding",
		{"clutches", "claws", "bites", {0}}},
	{"Parts of this hybrid of man and serpent are beautiful and beguiling, the rest are monstrous. $HESHE has scooped out $HISHER eyes like seeds from a pod and hidden them somewhere.",
		"savouring", "Feeding",
		{"licks", "kisses", "bites", {0}}},
	{"The zombie is the accursed product of a long-forgotten ritual. Perpetually decaying flesh, hanging from $HISHER bones in shreds, releases a putrid and flammable stench that will induce violent nausea in anyone who inhales it.",
		"rending", "Eating",
		{"hits", "bites", {0}}},
	{"An enormous, disfigured creature covered in phlegm and warts, the troll regenerates very quickly and attacks with astonishing strength. Many adventures have ended at $HISHER misshapen hands.",
		"eating", "Eating",
		{"cudgels", "clubs", "bludgeons", "pummels", "batters"}},
	{"This ogre is bent with age, but what $HESHE has lost in physical strength, $HESHE has more than gained in occult power.",
		"performing a ritual on", "Performing ritual",
		{"cudgels", "clubs", {0}},
		{0},
		"chants in a harsh, guttural tongue!"},
	{"The serpentine naga live beneath the subterranean waters and emerge to attack unsuspecting adventurers.",
		"studying", "Studying",
		{"claws", "bites", "tail-whips", {0}}},
	{"A serpent wreathed in flames and carrying a burning lash, salamanders dwell in lakes of fire and emerge when they sense a nearby victim, leaving behind a trail of glowing embers.",
		"studying", "Studying",
		{"claws", "whips", "lashes", {0}}},
	{"This rare subspecies of bloat is little more than a thin membrane surrounding a bladder of highly explosive gases. The slightest stress will cause $HIMHER to rupture in spectacular and deadly fashion.",
		"gazing at", "Gazing",
		{"bumps", {0}},
		"detonates with terrifying force!"},
	{"An elf of the deep, the dar blademaster leaps toward $HISHER enemies with frightening speed to engage in deadly swordplay.",
		"studying", "Studying",
		{"grazes", "cuts", "slices", "slashes", "stabs"}},
	{"The dar priestess carries a host of religious relics that jangle as $HESHE walks.",
		"praying over", "Praying",
		{"cuts", "slices", {0}}},
	{"The dar battlemage's eyes glow an angry shade of red, and $HISHER hands radiate an occult heat.",
		"transmuting", "Transmuting",
		{"cuts", {0}}},
	{"A jelly subsisting on a diet of acid mounds will eventually express the characteristics of $HISHER prey, corroding any weapons or armor that come in contact with $HIMHER.",
		"transmuting", "Transmuting",
		{"burns", {0}}},
	{"Half man and half horse, the centaur is an expert with the bow and arrow -- hunter and steed fused into a single creature.",
		"studying", "Studying",
		{"shoots", {0}}},
	{"A strange and horrifying creature of the earth's deepest places, larger than an ogre but capable of squeezing through tiny openings. When hungry, the underworm will burrow behind the walls of a cavern and lurk dormant and motionless -- often for months -- until $HESHE can feel the telltale vibrations of nearby prey.",
		"consuming", "Consuming",
		{"slams", "bites", "tail-whips", {0}}},	
	{"An ancient statue of an unrecognizable humanoid figure, the sentinel holds aloft a crystal that gleams with ancient warding magic. Sentinels are always found in groups of three, and each will attempt to repair any damage done to the other two.",
		"focusing on", "Focusing",
		{"hits", {0}}},
	{"A green-flecked nozzle is embedded in the wall, ready to spew a stream of corrosive acid at intruders.",
		"gazing at", "Gazing",
		{"douses", "drenches", {0}}},
	{"This spring-loaded contraption fires darts that are imbued with a strength-sapping poison.",
		"gazing at", "Gazing",
		{"pricks", {0}}},
	{"This tentacled nightmare will emerge from the subterranean waters to ensnare and devour any creature foolish enough to set foot into $HISHER lake.",
		"devouring", "Feeding",
		{"slaps", "smites", "batters", {0}}},
	{"The desiccated form of an ancient sorcerer animated by dark arts and lust for power, the lich commands the obedience of the infernal planes and their foul inhabitants. $HISHER essence is anchored to reality by a green phylactery that is always in $HISHER possession, and the lich cannot die unless the gem is destroyed.",
		"enchanting", "Enchanting",
		{"touches", {0}},
		{0},
		"rasps a terrifying incantation!"},
	{"This gem was the fulcrum of a dark rite, performed centuries ago, that bound the soul of an ancient and terrible sorcerer. Hurry and destroy the gem, before the lich can gather its power and regenerate its corporeal form!",
		"enchanting", "Enchanting",
		{"touches", {0}},
		{0},
		"swirls with dark sorcery as the lich regenerates its form!"},
	{"A peculiar airborne humanoid, the pixie can cause all manner of trouble with a variety of spells. What $HESHE lacks in physical endurance, $HESHE makes up for with $HISHER wealth of mischievous magical abilities.",
		"sprinkling dust on", "Dusting",
		{"pokes", {0}}},
	{"A silhouette of mournful rage against an empty backdrop, the phantom slips through the dungeon invisibly in clear air, leaving behind glowing droplets of ectoplasm and the cries of $HISHER unsuspecting victims.",
		"permeating", "Permeating",
		{"hits", {0}}},
	{"This infernal contraption spits blasts of flame at intruders.",
		"incinerating", "Incinerating",
		{"pricks", {0}}},
	{"This trickster demon moves with astonishing speed and delights in stealing from $HISHER enemies and blinking away.",
		"dissecting", "Dissecting",
		{"slices", "cuts", {0}}},
	{"A creature of inchoate rage made flesh, the fury's moist wings beat loudly in the darkness.",
		"flagellating", "Flagellating",
		{"drubs", "fustigates", "castigates", {0}}},
	{"This unholy specter stalks the deep places of the earth without fear, impervious to all conventional attacks.",
		"desecrating", "Desecrating",
		{"hits", {0}}},
	{"This seething, towering nightmare of fleshy tentacles slinks through the bowels of the world. The tentacle horror's incredible strength and regeneration make $HIMHER one of the most fearsome creatures of the dungeon.",
		"sucking on", "Consuming",
		{"slaps", "batters", "crushes", {0}}},
	{"A statue animated by a tireless and ancient magic, the golem does not regenerate and attacks with only moderate strength, but $HISHER stone form can withstand an incredible amount of damage before collapsing into rubble.",
		"cradling", "Cradling",
		{"backhands", "punches", "kicks", {0}}},
	{"An ancient serpent of the world's deepest places, the dragon's immense form belies its lightning-quick speed and testifies to $HISHER breathtaking strength. An undying furnace of white-hot flames burns within $HISHER scaly hide, and few could withstand a single moment under $HISHER infernal lash.",
		"consuming", "Consuming",
		{"claws", "bites", {0}}},
	{"An ancient serpent of the world's deepest places, the dragon's immense form belies its lightning-quick speed and testifies to $HISHER breathtaking strength. An undying furnace of white-hot flames burns within $HISHER scaly hide, and few could withstand a single moment under $HISHER infernal lash.",
		"consuming", "Consuming",
		{"claws", "bites", {0}}},
	{"Revenants have created this ornate totem and imbued $HIMHER with an unholy power.",
		"gazing at", "Gazing",
		{"hits", {0}}},
	{"The dar have created this engraved totem and imbued $HIMHER with an unearthly power.",
		"gazing at", "Gazing",
		{"hits", {0}}},
	{"Dragons have created this rough hewn totem and imbued $HIMHER with an ancient power.",
		"gazing at", "Gazing",
		{"hits", {0}}},

	{"Taller, stronger and smarter than other goblins, the warlord commands the loyalty of $HISHER kind and can summon them into battle.",
		"chanting over", "Chanting",
		{"slashes", "cuts", "stabs", {0}},
		{0},
		"lets loose a deafening war cry!"},
	{"This blob of jet-black goo is as rare as $HESHE is deadly. Few creatures of the dungeon can withstand $HISHER poisonous assault. Beware.",
		"absorbing", "Feeding",
		{"smears", "slimes", "drenches"}},
	{"This vampire lives a solitary life deep underground, consuming any warm-blooded creature unfortunate to venture near $HISHER lair.",
		"draining", "Drinking",
		{"grazes", "bites", "buries his fangs in", {0}},
		{0},
		"spreads his cloak and bursts into a cloud of bats!"},
	{"An elemental creature from another plane of existence, the infernal flamedancer burns with such intensity that $HESHE is painful to behold.",
		"immolating", "Consuming",
		{"singes", "burns", "immolates", {0}}},
	{"A bull-headed brute who has slipped $HISHER collar, snorts steam from $HISHER nostrils and paws at the ground with $HISHER massive hooves.",
		"butting", "Consuming",
		{"butts", "charges", "tramples", {0}}},

	{"An iron statue crackling with unearthly magics, the iron golem is much more dangerous than $HISHER brethren, cannot be killed by negation and casting powerful magics in addition to withstanding an incredible amount of damage.",
		"cradling", "Cradling",
		{"backhands", "punches", "kicks", {0}}},

	{"Eldritch forces have coalesced to form this flickering, ethereal weapon.",
		"gazing at", "Gazing",
		{"nicks",  {0}}},
	{"Mysterious energies bound up in your equipment have leapt forth to project this spectral image.",
		"gazing at", "Gazing",
		{"hits",  {0}}},
	{"Guarding the room is a weathered stone statue of a knight carrying a battleaxe, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
		"gazing at", "Gazing",
		{"strikes",  {0}}},
	{"A statue of a sword-wielding angel surveys the room, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
		"gazing at", "Gazing",
		{"strikes",  {0}}},
	{"This totem sits at the center of a summoning circle that radiates a strange energy.",
		"gazing at", "Gazing",
		{"strikes",  {0}},
        {0},
        "crackles with energy as you touch the glyph!"},
	{"A prism of shoulder-high mirrored surfaces gleams in the darkness.",
		"gazing at", "Gazing",
		{"strikes",  {0}}},
	{"$HESHE is half-formed and feverish, still dripping the fluid from the sac which gave $HISHER life.",
		"gazing at", "Gazing",
		{"wails at","shrieks at", {0}}},
	{"This soft, grey, fleshy mass of fungus is a dungeon delicacy. It can take a while to consume.",
		"growing in", "Growing in",
		{"releases spores at", {0}}},
	{"You can somehow sense this waxy mushroom and its immediate surroundings from across the dungeon.",
		"growing in", "Growing in",
		{"releases spores at", {0}}},

	{"The unicorn's flowing white mane and tail shine with rainbow light, $HISHER horn glows with healing and protective magic, and $HISHER eyes implore you to always chase your dreams. Unicorns are rumored to be attracted to virgins -- is there a hint of accusation in $HISHER gaze?",
		"consecrating", "Consecrating",
		{"pokes", "stabs", "gores", {0}}},
	{"A whirling desert storm given human shape, the ifrit's twin scimitars flicker and shine in the darkness and $HISHER eyes burn with otherworldly flame.",
		"absorbing", "Absorbing",
		{"cuts", "slashes", "lacerates", {0}}},
	{"This legendary bird shines with a brilliant light, and $HISHER wings crackle and pop like embers as they beat the air. When $HESHE dies, legend has it that an egg will form and a newborn phoenix will rise from its ashes.",
		"cremating", "Cremating",
		{"pecks", "scratches", "claws", {0}}},
	{"Cradled in a nest of cooling ashes, the translucent membrane of the phoenix egg reveals a yolk that glows ever brighter by the second.",
		"cremating", "Cremating",
		{"touches", {0}},
		{0},
		"bursts as a newborn phoenix rises from the ashes!"},

	{"", "", "", {{0}}},

	{"", "", "", {{0}}},

};

#pragma mark Horde definitions

const hordeType hordeCatalog[NUMBER_HORDES] = {
	// leader		#members	member list								member numbers					minL	maxL	freq	spawnsIn		machine			flags
	{MK_RAT,			0,		{0},									{{0}},							1,		5,		10},
	{MK_KOBOLD,			0,		{0},									{{0}},							1,		6,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_JACKAL,			0,		{0},									{{0}},							1,		3,		10},
	{MK_JACKAL,			1,		{MK_JACKAL},							{{1, 3, 1}},					3,		7,		5},
	{MK_EEL,			0,		{0},									{{0}},							2,		17,		10,		DEEP_WATER},
	{MK_MONKEY,			0,		{0},									{{0}},							2,		9,		7},
	{MK_BLOAT,			0,		{0},									{{0}},							2,		13,		3,		0,				0,				HORDE_BLOAT},
	{MK_PIT_BLOAT,		0,		{0},									{{0}},							2,		13,		1,		0,				0,				HORDE_BLOAT},
	{MK_BLOAT,			1,		{MK_BLOAT},								{{0, 2, 1}},					14,		26,		3,		0,				0,				HORDE_BLOAT},
	{MK_PIT_BLOAT,		1,		{MK_PIT_BLOAT},							{{0, 2, 1}},					14,		26,		1,		0,				0,				HORDE_BLOAT},
	{MK_EXPLOSIVE_BLOAT,0,		{0},									{{0}},							10,		26,		1,		0,				0,				HORDE_BLOAT},
	{MK_GOBLIN,			0,		{0},									{{0}},							3,		10,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_GOBLIN_CONJURER,0,		{0},									{{0}},							3,		10,		6,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_TOAD,			0,		{0},									{{0}},							4,		11,		10,		0,				0,				HORDE_TOAD},
	{MK_PINK_JELLY,		0,		{0},									{{0}},							4,		13,		10},
	{MK_GOBLIN_TOTEM,	1,		{MK_GOBLIN},							{{2,4,1}},						5,		13,		10,		0,				MT_CAMP_AREA,	HORDE_NO_PERIODIC_SPAWN},
	{MK_ARROW_TURRET,	0,		{0},									{{0}},							5,		13,		10,		TOP_WALL,	0,					HORDE_NO_PERIODIC_SPAWN | HORDE_SENTRY},
	{MK_MONKEY,			0,		{0},									{{0}},							5,		12,		5},
	{MK_MONKEY,			1,		{MK_MONKEY},							{{2,4,1}},						5,		13,		2},
    {MK_VAMPIRE_BAT,	0,		{0},                                    {{0}},                          6,		13,		3},
    {MK_VAMPIRE_BAT,	1,		{MK_VAMPIRE_BAT},						{{1,2,1}},						6,		13,		7,      0,          0,                  HORDE_NEVER_OOD},
	{MK_ACID_MOUND,		0,		{0},									{{0}},							6,		13,		10},
	{MK_GOBLIN,			3,		{MK_GOBLIN, MK_GOBLIN_MYSTIC, MK_JACKAL},{{2, 3, 1}, {1,2,1}, {1,2,1}},	6,		12,		4,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_GOBLIN_CONJURER,2,		{MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC},	{{0,1,1}, {1,1,1}},				7,		15,		4,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_CENTIPEDE,		0,		{0},									{{0}},							7,		14,		10},
	{MK_BOG_MONSTER,	0,		{0},									{{0}},							7,		14,		8,		MUD,        0,                  HORDE_NEVER_OOD},
	{MK_OGRE,			0,		{0},									{{0}},							7,		13,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_EEL,			1,		{MK_EEL},								{{2, 4, 1}},					8,		22,		7,		DEEP_WATER},
	{MK_ACID_MOUND,		1,		{MK_ACID_MOUND},						{{2, 4, 1}},					9,		13,		3},
	{MK_SPIDER,			0,		{0},									{{0}},							9,		16,		10,		0,				0,				HORDE_SPIDER},
	{MK_DAR_BLADEMASTER,1,		{MK_DAR_BLADEMASTER},					{{0, 1, 1}},					10,		14,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_WILL_O_THE_WISP,0,		{0},									{{0}},							10,		17,		10},
	{MK_WRAITH,			0,		{0},									{{0}},							10,		17,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_GOBLIN_TOTEM,	4,		{MK_GOBLIN_TOTEM, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN}, {{1,2,1},{1,2,1},{1,2,1},{3,5,1}},10,17,8,0,MT_CAMP_AREA,	HORDE_NO_PERIODIC_SPAWN},
	{MK_HARPY,			0,		{MK_HARPY},								{{1, 2, 1}},					11,		18,		4,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_SPARK_TURRET,	0,		{0},									{{0}},							11,		18,		10,		TOP_WALL,	0,					HORDE_NO_PERIODIC_SPAWN | HORDE_SENTRY},
	{MK_ZOMBIE,			0,		{0},									{{0}},							11,		18,		10,		0,				0,				HORDE_ZOMBIE},
	{MK_LAMIA,			0,		{0},									{{0}},							11,		18,		4,		DEEP_WATER},
	{MK_TROLL,			0,		{0},									{{0}},							12,		19,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_OGRE_TOTEM,		1,		{MK_OGRE},								{{2,4,1}},						12,		19,		6,		0,			0,					HORDE_NO_PERIODIC_SPAWN},
	{MK_BOG_MONSTER,	1,		{MK_BOG_MONSTER},						{{2,4,1}},						12,		26,		10,		MUD},
	{MK_NAGA,			0,		{0},									{{0}},							13,		20,		10,		DEEP_WATER,	0,					HORDE_NAGA | HORDE_ASSASSINATION_TARGET},
	{MK_SALAMANDER,		0,		{0},									{{0}},							13,		20,		10,		LAVA,		0,					HORDE_ASSASSINATION_TARGET},
	{MK_OGRE_SHAMAN,	1,		{MK_OGRE},								{{1, 3, 1}},					14,		20,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_CENTAUR,		1,		{MK_CENTAUR},							{{1, 1, 1}},					14,		21,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_ACID_JELLY,		0,		{0},									{{0}},							14,		21,		10},
	{MK_ACID_TURRET,	0,		{0},									{{0}},							15,		22,		10,		TOP_WALL,	0,					HORDE_NO_PERIODIC_SPAWN | HORDE_SENTRY},
    {MK_DART_TURRET,	0,		{0},									{{0}},							15,		22,		10,		TOP_WALL,	0,					HORDE_NO_PERIODIC_SPAWN | HORDE_SENTRY},
	{MK_PIXIE,			0,		{0},									{{0}},							14,		21,		8,		0,				0,				HORDE_PIXIE | HORDE_ASSASSINATION_TARGET},
	{MK_FLAME_TURRET,	0,		{0},									{{0}},							14,		24,		10,		TOP_WALL,	0,					HORDE_NO_PERIODIC_SPAWN | HORDE_SENTRY},
	{MK_DAR_BLADEMASTER,2,		{MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS},	{{0, 1, 1}, {0, 1, 1}},			15,		17,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_PINK_JELLY,     2,		{MK_PINK_JELLY, MK_DAR_PRIESTESS},      {{0, 1, 1}, {1, 2, 1}},			17,		23,		7},
	{MK_KRAKEN,			0,		{0},									{{0}},							15,		30,		10,		DEEP_WATER},
	{MK_PHANTOM,		0,		{0},									{{0}},							16,		23,		10,		0,				0,				/*HORDE_PHANTOM*/},
	{MK_WRAITH,			1,		{MK_WRAITH},							{{1, 4, 1}},					16,		23,		8,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_IMP,			0,		{0},									{{0}},							17,		24,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_DAR_BLADEMASTER,3,		{MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,10,	0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_FURY,			1,		{MK_FURY},								{{2, 4, 1}},					18,		26,		8},
	{MK_REVENANT,		0,		{0},									{{0}},							19,		27,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_KRAKEN,			0,		{MK_LAMIA},								{{2, 3, 1}},					20,		30,		3,		DEEP_WATER},
	{MK_GOLEM,			0,		{0},									{{0}},							21,		30,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_TENTACLE_HORROR,0,		{0},									{{0}},							22,		40,		10,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_PHYLACTERY,		0,		{0},									{{0}},							22,		45,		10},
	{MK_DRAGON,			0,		{0},									{{0}},							24,		50,		7},
	{MK_DRAGON,			1,		{MK_DRAGON},							{{1,1,1}},						27,		50,		3},
	{MK_GOLEM,			3,		{MK_GOLEM, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE}, {{1, 2, 1}, {0,1,1},{0,1,1}},27,99,	8},
	{MK_GOLEM,			1,		{MK_GOLEM},								{{5, 10, 2}},					30,		99,     2},
	{MK_KRAKEN,			1,		{MK_KRAKEN},							{{5, 10, 2}},					30,		99,		10,		DEEP_WATER},
	{MK_TENTACLE_HORROR,2,		{MK_TENTACLE_HORROR, MK_REVENANT},		{{1, 3, 1}, {2, 4, 1}},			32,		99,     2},
	{MK_DRAGON,			1,		{MK_DRAGON},							{{3, 5, 1}},					34,		99,     2},
	{MK_SHY_DRAGON,		1,		{MK_SHY_DRAGON},						{{1,1,1}},						37,		50,		5,		0,				0,				HORDE_ASSASSINATION_TARGET},
	{MK_BLACK_JELLY,    2,		{MK_BLACK_JELLY, MK_DAR_PRIESTESS},     {{1, 2, 1}, {2, 3, 1}},			37,		43,		7},
	{MK_REVENANT_TOTEM,	1,		{MK_REVENANT},							{{2,4,1}},						39,		99,		5,		0,				MT_CAMP_AREA,	HORDE_NO_PERIODIC_SPAWN},
	{MK_SHY_DRAGON,		1,		{MK_SHY_DRAGON},						{{3, 5, 1}},					44,		99,     2},
	{MK_DAR_TOTEM,		1,		{MK_DAR_PRIESTESS},						{{2,4,1}},						44,		99,		5,		0,				MT_CAMP_AREA,	HORDE_NO_PERIODIC_SPAWN},
	{MK_DRAGON_TOTEM,	1,		{MK_DRAGON},							{{2,4,1}},						49,		99,		5,		0,				MT_DISMAL_AREA,	HORDE_NO_PERIODIC_SPAWN},

	// underworms never appear naturally but can be summoned by a wand/staff
	{MK_UNDERWORM,		0,		{0},									{{0}},							0,		0,		10,		0,			0,					HORDE_UNDERWORM},

	// summons
	{MK_GOBLIN_CONJURER,1,		{MK_SPECTRAL_BLADE},					{{3, 5, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
	{MK_OGRE_SHAMAN,	1,		{MK_OGRE},								{{1, 1, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_VAMPIRE,		1,		{MK_VAMPIRE_BAT},						{{3, 3, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_LICH,			1,		{MK_PHANTOM},							{{2, 3, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_LICH,			1,		{MK_FURY},								{{2, 3, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_PHYLACTERY,		1,		{MK_LICH},								{{1,1,1}},						0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_GOBLIN_CHIEFTAN,2,		{MK_GOBLIN_CONJURER, MK_GOBLIN},		{{1,1,1}, {2,3,1}},				0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
	{MK_PHOENIX_EGG,	1,		{MK_PHOENIX},							{{1,1,1}},						0,		0,		10,		0,			0,					HORDE_IS_SUMMONED},
    {MK_ELDRITCH_TOTEM, 1,		{MK_SPECTRAL_BLADE},					{{4, 7, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
    {MK_ELDRITCH_TOTEM, 1,		{MK_FURY},                              {{2, 3, 1}},					0,		0,		10,		0,			0,					HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
	{MK_DAR_TOTEM,		1,		{MK_DAR_BLADEMASTER},					{{2,3,1}},						0,		0,		10,		0,			0,					HORDE_NO_PERIODIC_SPAWN},
	{MK_DAR_TOTEM,		1,		{MK_DAR_BATTLEMAGE},					{{1,2,1}},						0,		0,		10,		0,			0,					HORDE_NO_PERIODIC_SPAWN},

	// captives
	{MK_MONKEY,			1,		{MK_KOBOLD},							{{1, 2, 1}},					1,		5,		1,		0,			0,					HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN,			1,		{MK_GOBLIN},							{{1, 2, 1}},					3,		7,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_OGRE,			1,		{MK_GOBLIN},							{{3, 5, 1}},					4,		10,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_GOBLIN_MYSTIC,	1,		{MK_KOBOLD},							{{3, 7, 1}},					5,		11,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_OGRE,			1,		{MK_OGRE},								{{1, 2, 1}},					8,		15,		2,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_TROLL,			1,		{MK_TROLL},								{{1, 2, 1}},					12,		19,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_CENTAUR,		1,		{MK_TROLL},								{{1, 2, 1}},					12,		19,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_TROLL,			2,		{MK_OGRE, MK_OGRE_SHAMAN},				{{2, 3, 1}, {0, 1, 1}},			14,		19,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_DAR_BLADEMASTER,1,		{MK_TROLL},								{{1, 2, 1}},					12,		19,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_NAGA,			1,		{MK_SALAMANDER},						{{1, 2, 1}},					13,		20,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_SALAMANDER,		1,		{MK_NAGA},								{{1, 2, 1}},					13,		20,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_TROLL,			1,		{MK_SALAMANDER},						{{1, 2, 1}},					13,		19,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_IMP,			1,		{MK_FURY},								{{2, 4, 1}},					18,		26,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_PIXIE,			1,		{MK_IMP, MK_PHANTOM},					{{1, 2, 1}, {1, 2, 1}},			14,		21,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_DAR_BLADEMASTER,1,		{MK_FURY},								{{2, 4, 1}},					18,		26,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_DAR_BLADEMASTER,1,		{MK_IMP},								{{2, 3, 1}},					18,		26,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_DAR_PRIESTESS,	1,		{MK_FURY},								{{2, 4, 1}},					18,		26,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_DAR_BATTLEMAGE,	1,		{MK_IMP},								{{2, 3, 1}},					18,		26,		1,		0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_TENTACLE_HORROR,3,		{MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,1,	0,			0,					HORDE_LEADER_CAPTIVE | HORDE_ASSASSINATION_TARGET},
	{MK_GOLEM,			3,		{MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,1,	0,			0,					HORDE_LEADER_CAPTIVE},
	
	// bosses
	{MK_GOBLIN_CHIEFTAN,2,		{MK_GOBLIN, MK_GOBLIN_TOTEM},			{{2,3,1}, {2,2,1}},				2,		10,		5,		0,			0,					HORDE_MACHINE_BOSS},
	{MK_BLACK_JELLY,	0,		{0},									{{0}},							5,		15,		5,		0,			0,					HORDE_MACHINE_BOSS},
	{MK_VAMPIRE,		0,		{0},									{{0}},							10,		100,	5,		0,			0,					HORDE_MACHINE_BOSS},
	{MK_FLAMESPIRIT,	0,		{0},									{{0}},							10,		100,	5,		0,			0,					HORDE_MACHINE_BOSS},
	{MK_MINOTAUR,		0,		{0},									{{0}},							10,		100,	5,		0,			0,					HORDE_MACHINE_BOSS},
	
	// machine water monsters
	{MK_EEL,			0,		{0},									{{0}},							2,		7,		10,		DEEP_WATER,	0,					HORDE_MACHINE_WATER_MONSTER},
	{MK_EEL,			1,		{MK_EEL},								{{2, 4, 1}},					5,		15,		10,		DEEP_WATER,	0,					HORDE_MACHINE_WATER_MONSTER},
	{MK_KRAKEN,			0,		{0},									{{0}},							12,		100,	10,		DEEP_WATER,	0,					HORDE_MACHINE_WATER_MONSTER},
	{MK_KRAKEN,			1,		{MK_EEL},								{{1, 2, 1}},					12,		100,	8,		DEEP_WATER,	0,					HORDE_MACHINE_WATER_MONSTER},
	
	// dungeon captives -- no captors
	{MK_OGRE,			0,		{0},									{{0}},							1,		5,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_NAGA,			0,		{0},									{{0}},							2,		8,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN_MYSTIC,	0,		{0},									{{0}},							2,		8,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_TROLL,			0,		{0},									{{0}},							5,		11,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_DAR_BLADEMASTER,0,		{0},									{{0}},							8,		14,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_DAR_PRIESTESS,	0,		{0},									{{0}},							8,		14,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_WRAITH,			0,		{0},									{{0}},							11,		17,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_GOLEM,			0,		{0},									{{0}},							17,		23,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_TENTACLE_HORROR,0,		{0},									{{0}},							20,		26,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	{MK_DRAGON,			0,		{0},									{{0}},							23,		26,		10,		0,			0,					HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
	
	// machine statue monsters
	{MK_GOBLIN,			0,		{0},									{{0}},							1,		6,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	{MK_OGRE,			0,		{0},									{{0}},							6,		12,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	{MK_WRAITH,			0,		{0},									{{0}},							10,		17,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	{MK_NAGA,			0,		{0},									{{0}},							12,		19,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	{MK_TROLL,			0,		{0},									{{0}},							14,		21,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	{MK_GOLEM,			0,		{0},									{{0}},							21,		30,		10,		STATUE_DORMANT, 0,				HORDE_MACHINE_STATUE},
	
	// machine turrets
	{MK_ARROW_TURRET,	0,		{0},									{{0}},							5,		13,		10,		TURRET_DORMANT, 0,				HORDE_MACHINE_TURRET},
	{MK_SPARK_TURRET,	0,		{0},									{{0}},							11,		18,		10,		TURRET_DORMANT, 0,				HORDE_MACHINE_TURRET},
	{MK_ACID_TURRET,	0,		{0},									{{0}},							15,		22,		10,		TURRET_DORMANT, 0,				HORDE_MACHINE_TURRET},
	{MK_DART_TURRET,	0,		{0},									{{0}},							15,		22,		10,		TURRET_DORMANT, 0,				HORDE_MACHINE_TURRET},
	{MK_FLAME_TURRET,	0,		{0},									{{0}},							17,		24,		10,		TURRET_DORMANT, 0,				HORDE_MACHINE_TURRET},
	
	// machine mud monsters
	{MK_BOG_MONSTER,	0,		{0},									{{0}},							12,		26,		10,		MACHINE_MUD_DORMANT, 0,			HORDE_MACHINE_MUD},
	{MK_KRAKEN,			0,		{0},									{{0}},							17,		26,		3,		MACHINE_MUD_DORMANT, 0,			HORDE_MACHINE_MUD},
	
	// kennel monsters
	{MK_MONKEY,			0,		{0},									{{0}},							1,		5,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN,			0,		{0},									{{0}},							1,		8,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN_CONJURER,0,		{0},									{{0}},							2,		9,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN_MYSTIC,	0,		{0},									{{0}},							2,		9,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_OGRE,			0,		{0},									{{0}},							5,		15,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_TROLL,			0,		{0},									{{0}},							10,		19,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_NAGA,			0,		{0},									{{0}},							9,		20,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_SALAMANDER,		0,		{0},									{{0}},							9,		20,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_IMP,			0,		{0},									{{0}},							15,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_PIXIE,			0,		{0},									{{0}},							11,		21,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},	
	{MK_DAR_BLADEMASTER,0,		{0},									{{0}},							9,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_DAR_PRIESTESS,	0,		{0},									{{0}},							12,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	{MK_DAR_BATTLEMAGE,	0,		{0},									{{0}},							13,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
	
	// vampire bloodbags
	{MK_MONKEY,			0,		{0},									{{0}},							1,		5,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN,			0,		{0},									{{0}},							1,		8,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN_CONJURER,0,		{0},									{{0}},							2,		9,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_GOBLIN_MYSTIC,	0,		{0},									{{0}},							2,		9,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_OGRE,			0,		{0},									{{0}},							5,		15,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_TROLL,			0,		{0},									{{0}},							10,		19,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_NAGA,			0,		{0},									{{0}},							9,		20,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_IMP,			0,		{0},									{{0}},							15,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_PIXIE,			0,		{0},									{{0}},							11,		21,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},	
	{MK_DAR_BLADEMASTER,0,		{0},									{{0}},							9,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_DAR_PRIESTESS,	0,		{0},									{{0}},							12,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	{MK_DAR_BATTLEMAGE,	0,		{0},									{{0}},							13,		26,		10,		MONSTER_CAGE_CLOSED, 0,			HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
	
	// legendary allies
	{MK_UNICORN,		0,		{0},									{{0}},							1,		100,	10,		0,			0,					HORDE_MACHINE_LEGENDARY_ALLY},
	{MK_IFRIT,			0,		{0},									{{0}},							1,		100,	10,		0,			0,					HORDE_MACHINE_LEGENDARY_ALLY},
	{MK_PHOENIX_EGG,	0,		{0},									{{0}},							1,		100,	10,		0,			0,					HORDE_MACHINE_LEGENDARY_ALLY},

	// arena fighters - matches kennel monsters above - significantly OOD
	{MK_OGRE,			0,		{0},									{{0}},							5,		15,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_TROLL,			0,		{0},									{{0}},							10,		19,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_NAGA,			0,		{0},									{{0}},							9,		20,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_SALAMANDER,		0,		{0},									{{0}},							9,		20,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_IMP,			0,		{0},									{{0}},							15,		26,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_PIXIE,			0,		{0},									{{0}},							11,		21,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},	
	{MK_DAR_BLADEMASTER,0,		{0},									{{0}},							9,		26,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_DAR_PRIESTESS,	0,		{0},									{{0}},							12,		26,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},
	{MK_DAR_BATTLEMAGE,	0,		{0},									{{0}},							13,		26,		10,		ARENA_CAGE_CLOSED, 0,			HORDE_MACHINE_ARENA | HORDE_LEADER_CAPTIVE},

	// sticky bundle contents - monsters which are in some way 'hard to kill' - 4 levels OOD
	{MK_ZOMBIE,			0,		{0},									{{0}},							7,		14,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_TROLL,			0,		{0},									{{0}},							8,		15,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_PHANTOM,		0,		{0},									{{0}},							12,		19,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_REVENANT,		0,		{0},									{{0}},							15,		23,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_GOLEM,			0,		{0},									{{0}},							17,		26,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_TENTACLE_HORROR,0,		{0},									{{0}},							18,		36,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},
	{MK_PHYLACTERY,		0,		{0},									{{0}},							18,		41,		10,		STICKY_BUNDLE_DORMANT, 0,		HORDE_MACHINE_BUNDLE},

	// prism contents - really dangerous monsters - up to 9 levels OOD
	{MK_REVENANT,		0,		{0},									{{0}},							10,		18,		10,		ALTAR_INERT,	0,				HORDE_MACHINE_PRISM},
	{MK_GOLEM,			0,		{0},									{{0}},							13,		22,		10,		ALTAR_INERT,	0,				HORDE_MACHINE_PRISM},
	{MK_TENTACLE_HORROR,0,		{0},									{{0}},							15,		33,		10,		ALTAR_INERT,	0,				HORDE_MACHINE_PRISM},
	{MK_DRAGON,			0,		{0},									{{0}},							17,		44,		7,		ALTAR_INERT,	0,				HORDE_MACHINE_PRISM},

	// chamber contents - dangerous monsters raised from the dead - 6 levels OOD
	{MK_ZOMBIE,			0,		{0},									{{0}},							7,		12,		10,		CHAMBER_CLOSED, 0,				HORDE_MACHINE_CHAMBER},
	{MK_PHANTOM,		0,		{0},									{{0}},							10,		17,		10,		CHAMBER_CLOSED, 0,				HORDE_MACHINE_CHAMBER},
	{MK_REVENANT,		0,		{0},									{{0}},							13,		21,		10,		CHAMBER_CLOSED, 0,				HORDE_MACHINE_CHAMBER},
	{MK_GOLEM,			0,		{0},									{{0}},							15,		24,		10,		CHAMBER_CLOSED, 0,				HORDE_MACHINE_CHAMBER},
	{MK_PHYLACTERY,		0,		{0},									{{0}},							16,		39,		10,		CHAMBER_CLOSED, 0,				HORDE_MACHINE_CHAMBER},

	// thieves
	{MK_MONKEY,			0,		{0},									{{0}},							1,		12,		7,		0,			0,					HORDE_MACHINE_THIEF},
	{MK_MONKEY,			1,		{MK_MONKEY},							{{2, 4, 1}},					5,		13,		2,		0,			0,					HORDE_MACHINE_THIEF},
	{MK_HARPY,			1,		{MK_HARPY},								{{1, 2, 1}},					11,		18,		8,		0,			0,					HORDE_MACHINE_THIEF},
	{MK_IMP,			0,		{0},									{{0}},							17,		24,		10,		0,			0,					HORDE_MACHINE_THIEF},

	// mandrakes
	{MK_MANDRAKE,		0,		{0},									{{0}},							1,		100,	1,		0,			0,					HORDE_MACHINE_MANDRAKE},

	// dead man's ear -- never naturally appears but can be summoned by wand/staff
	{MK_DEAD_MANS_EAR,	0,		{0},									{{0}},							0,		0,		0,		0,			0,					HORDE_DEAD_MANS_EAR},

	// crimson caps
	{MK_CRIMSON_CAP,	0,		{0},									{{0}},							1,		100,	1,		0,			0,					HORDE_MACHINE_CRIMSON_CAP},

	// mirrored totems -- never naturally appears but can be summoned by wand/staff
	{MK_MIRRORED_TOTEM,	0,		{0},									{{0}},							0,		0,		0,		0,			0,					HORDE_MIRRORED_TOTEM},

};

// LEVELS

const levelProfile levelProfileCatalog[NUMBER_LEVEL_PROFILES] = {
	//cave?		cross?	corrid?	door?	maxRms	maxLoops
	{33,		100,	80,		60,		99,		30},
};

// ITEMS

#pragma mark Item flavors

char itemTitles[NUMBER_SCROLL_KINDS][30];

const char titlePhonemes[NUMBER_TITLE_PHONEMES][30] = {
	"glorp",
	"snarg",
	"gana",
	"flin",
	"herba",
	"pora",
	"nuglo",
	"greep",
	"nur",
	"lofa",
	"poder",
	"nidge",
	"pus",
	"wooz",
	"flem",
	"bloto",
	"porta"
};

char itemCovers[NUMBER_SCROLL_KINDS][30];

const char coverPhrases[NUMBER_TITLE_PHONEMES][30] = {
	"proto",
	"synchro",
	"hemo",
	"tesla",
	"tronic",
	"celest",
	"mecha",
	"dyna",
	"edison",
	"loop",
	"coil",
	"globin",
	"whir",
	"back",
	"flow",
	"trix",
	"field"
};

char itemColors[NUMBER_ITEM_COLORS][30];

const char itemColorsRef[NUMBER_ITEM_COLORS][30] = {
	"crimson",
	"scarlet",
	"orange",
	"yellow",
	"green",
	"blue",
	"indigo",
	"violet",
	"puce",
	"mauve",
	"burgundy",
	"turquoise",
	"aquamarine",
	"gray",
	"pink",
	"white",
	"lavender",
	"tan",
	"brown",
	"cyan",
	"sea green",
	"lilac",
	"purple",
	"ivory",
	"chartreuse",
	"verdigris",
	"viridian",
	"azure"
};

char itemChemistry[NUMBER_ITEM_CHEMISTRY][30];

const char itemChemistryRef[NUMBER_ITEM_CHEMISTRY][30] = {
	"cloudy",
	"swirling",
	"hazy",
	"metallic",
	"misty",
	"bubbling",
	"foaming",
	"boiling",
	"syrupy",
	"steaming",
	"thick",
	"fizzing",
	"effervescent",
	"coagulated",
	"swirling",
	"clear",
	"blotchy",
	"layered",
	"incandescent",
	"icy",
	"cool",
	"unstable",
	"fermented",
	"oily",
	"curdled",
	"tepid",
	"briny",
	"simmering"
};

char itemWoods[NUMBER_ITEM_WOODS][30];

const char itemWoodsRef[NUMBER_ITEM_WOODS][30] = {
	"applewood",
	"aspen",
	"balsa",
	"bamboo",
	"banyan",
	"birch",
	"butternut",
	"cedar",
	"cottonwood",
	"cherry",
	"cypress",
	"dogwood",
	"elm",
	"eucalyptus",
	"fir",
	"hawthorn",
	"hemlock",
	"hickory",
	"ironwood",
	"laurel",
	"lebethron",
	"linden",
	"locust",
	"mahogany",
	"maple",
	"mistletoe",
	"mulberry",
	"oak",
	"olive",
	"palmwood",
	"pearwood",
	"pine",
	"poplar",
	"redwood",
	"rosewood",
	"rowan",
	"sandalwood",
	"sequoia",
	"spruce",
	"sycamore",
	"teak",
	"walnut",
	"willow",
	"yew"
};

char itemMetals[NUMBER_ITEM_METALS][30];

const char itemMetalsRef[NUMBER_ITEM_METALS][30] = {
	"aluminum",
	"antimony",
	"billon",
	"beryllium",
	"brass",
	"bronze",
	"cast iron",
	"chromium",
	"cobalt",
	"copper",
	"electrum",
	"galvorn",
	"gold",
	"hepatizon",
	"lead",
	"magnesium",
	"molybdenum",
	"nickel",
	"osmium",
	"palladium",
	"pewter",
	"platinum",
	"rhenium",
	"rhodite",
	"rhodium",
	"silver",
	"steel",
	"tantalum",
	"tin",
	"titanium",
	"tumbaga",
	"tungsten",
	"white gold",
	"wrought iron",
	"zirconium",
	"zinc"
};

char itemGems[NUMBER_ITEM_GEMS][30];

const char itemGemsRef[NUMBER_ITEM_GEMS][30] = {
	"agate",
	"alexandrite",
	"amethyst",
	"aquamarine",
	"azurite",
	"beryl",
	"bloodstone",
	"calcite",
	"carnelium",
	"corundum",
	"diamond",
	"emerald",
	"fluorite",
	"hematite",
	"jade",
	"jasper",
	"garnet",
	"lapis lazuli",
	"malachite",
	"marble",
	"moonstone",
	"nephrite",
	"obsidian",
	"onyx",
	"opal",
	"pearl",
	"quartz",
	"quartzite",
	"rose quartz",
	"rhodonite",
	"ruby",
	"sapphire",
	"tanzanite",
	"tiger eye",
	"topaz",
	"tourmaline",
	"turquoise",
	"zircon"
};

char itemMaterials[NUMBER_ITEM_MATERIALS][30];

const char itemMaterialsRef[NUMBER_ITEM_MATERIALS][30] = {
	"beaded",
	"feathered",
	"crystalline",
	"shattered",
	"handcarved",
	"granite",
	"chitinous",
	"weathered",
	"candied",
	"enameled",
	"faceted",
	"shrunken",
	"pickled",
	"magnetic",
	"twisted",
	"knotted",
	"tattooed",
	"inkstained",
	"frayed",
	"patchwork",
	"clockwork",
	"bloodstained",
	"barbed"
};

#pragma mark Item definitions

//typedef struct itemTable {
//	char *name;
//	char *flavor;
//	short frequency;
//	short marketValue;
//	short number;
//	randomRange range;
//} itemTable;

const itemTable keyTable[NUMBER_KEY_TYPES] = {
	{"door key",			"", "", 1, 0,	0, {0,0,0}, true, false, "The notches on this ancient iron key are well worn; its leather lanyard is battered by age. What door might it open?"},
	{"vault key",			"", "", 1, 0,	0, {0,0,0}, true, false, "This heavy gold scarab sits on top a key with teeth cut from precious gemstones. The door it opens must be protect something even more valuable."},
	{"cage key",			"", "", 1, 0,	0, {0,0,0}, true, false, "The rust accreted on this iron key has been stained with flecks of blood; it must have been used recently. What cage might it open?"},
	{"crystal orb",			"", "", 1, 0,	0, {0,0,0}, true, false, "A faceted orb, seemingly cut from a single crystal, sparkling and perpetually warm to the touch. What manner of device might such an object activate?"},
	{"armor rune",			"", "", 1, 0,	0, {0,0,0}, true, false, "A runestone made from meteoric iron; it shines faintly in the dark. What armor should you stamp with it?"},
	{"weapon rune",			"", "", 1, 0,	0, {0,0,0}, true, false, "A runestone made from a radiant diamond; its edges are razor sharp. What weapon should you mark with it?"},
	{"wand rune",			"", "", 1, 0,	0, {0,0,0}, true, false, "A runestone made from unusual alloys; it is icy cold to touch. What wand should you etch with it?"},
	{"staff rune",			"", "", 1, 0,	0, {0,0,0}, true, false, "A runestone made from petrified heartwood; smoothed down with age. What staff should you hew with it?"},
};

const itemTable foodTable[NUMBER_FOOD_KINDS] = {
	{"ration of food",		"", "", 3, 25,	1800, {0,0,0}, true, false, "A ration of food. Was it left by former adventurers? Is it a curious byproduct of the subterranean ecosystem?"},
	{"mango",				"", "", 1, 15,	1550, {0,0,0}, true, false, "An odd fruit to be found so deep beneath the surface of the earth, but only slightly less filling than a ration of food."}
};

const itemTable weaponTable[NUMBER_WEAPON_KINDS] = {
	{"dagger",				"", "", 10, 190,		10,	{3,	4,	1},		true, false, "A simple iron dagger with a well-worn wooden handle. "},
	{"sword",				"", "", 10, 440,		14, {6,	10,	1},		true, false, "The razor-sharp length of steel blade shines reassuringly. "},
	{"broadsword",			"", "", 10, 990,		19,	{14, 22, 1},	true, false, "This towering blade inflicts heavy damage by investing its heft into every cut. "},
	
	{"rapier",				"", "", 10, 440,		15, {3,	5,	1},		true, false, "This blade is thin and flexible, intended for deft and rapid maneuvers. It inflicts less damage than comparable weapons, but permits you to attack twice as quickly. If there is one space between you and an enemy and you step directly toward it, you will perform a devastating lunge attack, which deals treble damage and never misses. "},
	{"sabre",				"", "", 10, 990,		19, {4,	7,	1},		true, false, "A curved, single edged blade, intended for quickly slashing at all around you. It inflicts less damage than comparable weapons, but permits you to attack twice as quickly at all adjacent enemies simultaneously. If there is one space between you and an enemy and you step directly toward it, you will perform a devastating sweep attack, which deals treble damage to every adjacent enemy and never misses. "},

	{"mace",				"", "", 10, 660,		16, {9, 15, 1},		true, false, "The symmetrical iron flanges at the head of this weapon knocks back anything it hits two squares doing triple damage if the target is knocked into a wall, but it requires two turns to attack because of its weight. "},
	{"war hammer",			"", "", 10, 1100,		20, {15, 25, 1},	true, false, "The crushing blow of this towering mass of lead and steel knocks back anything it hits two squares doing tripled damage if the target is knocked into a wall, but only the strongest of adventurers can use it effectively, and it requires two turns to attack because of its weight. "},
	
	{"spear",				"", "", 10, 330,		13, {4, 5, 1},		true, false, "A slender wooden rod tipped with sharpened iron. The reach of the spear permits you to simultaneously attack an adjacent enemy and the enemy directly behind it. When you step towards an unoccupied grid you extend the spear into it, impaling any enemy which enters the grid for triple damage. "},
	{"war pike",			"", "", 10, 880,		18, {9, 15, 1},		true, false, "A long steel pole ending in a razor-sharp point. The reach of the pike permits you to simultaneously attack an adjacent enemy and the enemy directly behind it. When you step towards an unoccupied grid you extend the pike into it, impaling any enemy which enters the grid for triple damage. "},

	{"axe",					"", "", 10, 550,		15, {6, 9, 1},		true, false, "The blunt iron edge on this axe glints in the darkness. The arc of its swing permits you to attack all adjacent enemies simultaneously. "},
	{"war axe",				"", "", 10, 990,		19, {10, 17, 1},	true, false, "The enormous steel head of this war axe puts considerable heft behind each stroke. The arc of its swing permits you to attack all adjacent enemies simultaneously. "},

	{"dart",				"", "",	0,	15,			10,	{2,	4,	1},		true, false, "These simple metal spikes are weighted to fly true and sting their prey with a flick of the wrist. "},
	{"incendiary dart",		"", "",	7, 25,			12,	{1,	2,	1},		true, false, "The spike on each of these darts is designed to pin it to its target while the unstable compounds strapped to its length burst into brilliant flames. "},
	{"poison dart",			"", "",	0, 25,			12,	{3,	6,	1},		true, false, "The spike on this dart is coated with a poisonous substance which damages any target it penetrates. "},
	{"discord dart",		"", "",	0, 25,			12,	{2,	4,	1},		true, false, "The toxins coating this dart enrage the target and cause it to attack anything near by. "},
	{"confusing dart",		"", "",	0, 25,			12,	{2,	4,	1},		true, false, "The scintillating colours in the feathers decorating this dart create a confusing pattern which bedazzles anything it strikes. "},
	{"darkness dart",		"", "",	0, 25,			12,	{1,	2,	1},		true, false, "The hollow shaft of this delicate looking dart releases a cloud of black powder which dims the surrounding light. "},
	{"marking dart",		"", "",	0, 25,			12,	{0,	0,	0},		true, false, "This dart attaches itself surreptitiously to the target and sends out a telepathic signal alerting you to its location. "},
	{"weakness dart",		"", "",	0, 25,			12,	{3,	6,	1},		true, false, "The viscious looking barbs on this dart weaken anything it embeds into. "},
	{"slowing dart",		"", "",	0, 25,			12,	{2,	4,	1},		true, false, "The sticky substance which coats this dart slows anything it hits. "},
	{"grappling dart",		"", "",	3, 25,			15,	{3,	6,	1},		true, false, "This hooked dart is connected to a length of chain which is somehow as light as a feather. "},
	{"extinguishing dart",	"", "", 0, 25,			12, {2, 4,  1},		true, false, "This dart expells a fire suppressing gas on impact which extinguishes any nearby source of flame. It does triple damage to fiery monsters."},
	{"javelin",				"", "",	10, 40,			15,	{3, 11, 3},		true, false, "This length of metal is weighted to keep the spike at its tip foremost as it sails through the air. "},
};

const itemTable armorTable[NUMBER_ARMOR_KINDS] = {
	{"leather armor",	"", "", 10,	250,		10,	{30,30,1},		true, false, "This lightweight armor offers basic protection. It is the quietest of any armor. "},
	{"scale mail",		"", "", 10, 350,		12, {50,50,1},		true, false, "Bronze scales cover the surface of treated leather, offering greater protection than plain leather with minimal additional weight. It is not quite as stealthy as leather armor however."},
	{"chain mail",		"", "", 10, 500,		13, {60,60,1},		true, false, "Interlocking metal links make for a tough but flexible suit of armor. It does not affect your stealth, either positively or negatively."},
	{"banded mail",		"", "", 5, 800,			15, {80,80,1},		true, false, "Overlapping strips of metal horizontally encircle a chain mail base, offering an additional layer of protection at the cost of greater weight. The noise from this armor makes you easier to detect."},
	{"splint mail",		"", "", 5, 1000,		17, {100,100,1},	true, false, "Thick plates of metal are embedded into a chain mail base, providing the wearer with substantial protection. This armor is noisier than banded mail, but not as loud as plate armor."},
	{"plate armor",		"", "", 5, 1300,		20, {120,120,1},	true, false, "Enormous plates of metal are joined together into a suit that provides unmatched protection to any adventurer strong enough to bear its staggering weight. It is the noisiest of any armor."}
};

const itemTable shieldTable[NUMBER_SHIELD_KINDS] = { // blows until destroyed, base chance of taking no damage, minimum damage to count as destructive blow
	{"round shield",	"", "", 8,	350,		12,	{8,25,8},		true, false, "This round shield is made from planks of wood riveted together with a metal rim. "},
	{"knight's shield",	"", "", 5,	750,		16,	{12,40,12},		true, false, "This three peaked shield is made from wood reinforced with a thin metal shell. You can aim attacks which reflect off this shield."},
	{"tower shield",	"", "", 2,	1150,		20,	{16,55,16},		true, false, "This rectangular shield is as tall as neck to knee and provides unparalleled protection. It blocks slowing and webs in addition to lightning and fire -- these do not damage the shield."},
};

const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30] = {
	"speed",
	"quietus",
	"paralysis",
	"multiplicity",
	"slowing",
	"confusion",
	"force",
	"slaying",
	"mercy",
	"plenty",
};

const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30] = {
	"multiplicity",
	"mutuality",
	"absorption",
	"reprisal",
	"immunity",
	"reflection",
	"dampening",
	"burden",
	"vulnerability",
    "immolation",
};

boltRunic boltRunicCatalog[NUMBER_BOLT_ENCHANT_KINDS] = {
	{"quick", 0.5,	0,	(BOLT_QUICKLY), (BOLT_QUICKLY | BOLT_SLOWLY),
		"The quick rune makes $WANDORSTAFF twice as quick to use, but bolts only have half of their usual power. "},
	{"intense", 2.2,	0,	(BOLT_SLOWLY), (BOLT_QUICKLY | BOLT_SLOWLY),
		"The intense rune makes $WANDORSTAFF take two turns to use, but bolts have 220% of their usual power. "},
	{"receptive", 1.0,	0,	(BOLT_EASY_CHARGING), (BOLT_EASY_CHARGING),
		"The receptive rune makes $WANDORSTAFF charge twice as effectively. "},
	{"selective", 1.0,	0,	(BOLT_SELECTIVE), (BOLT_SELECTIVE | BOLT_BOUNCES),
		"The selective rune makes the bolts from $WANDORSTAFF not take effect until the grid you target, passing harmlessly through intervening monsters. "},
	{"precise",		1.0,	0,	(BOLT_PRECISION), (BOLT_PRECISION | BOLT_BOUNCES),
		"The precise rune makes the bolts from $WANDORSTAFF stop at the grid you target. "},
	{"focused",		1.5,	0,	(BOLT_DISTANCE), (BOLT_DISTANCE),
		"The focused rune makes bolts from $WANDORSTAFF have a limited range, but 150% of their usual power. "},		// blinking fires a force bolt first with this flag
	{"phasing",		1.0,	0,	(BOLT_X_RAY | BOLT_PREVENT_CONJURING | BOLT_PREVENT_ASSISTING), (BOLT_X_RAY),
		"The phasing rune makes bolts from $WANDORSTAFF pass through a single monster or obstruction, including walls! "},
	{"multiple",	0.6,	1,	(BOLT_PREVENT_UNUSUAL | BOLT_PREVENT_BECKONING | BOLT_PREVENT_POLYMORPH | BOLT_PREVENT_PLENTY), 0, 
		"The multiple rune lets you fire an additional bolt from $WANDORSTAFF every time you use it, without expending an additional charge. Bolts from the multiple staff only have 60% of their usual power. "},
	{"chaining",	1.0,	2,	(BOLT_CHAINING | BOLT_PREVENT_UNUSUAL),	(BOLT_CHAINING),
		"The chaining rune makes bolts from $WANDORSTAFF arc from your initial target to two additional targets nearby, including yourself if you are close enough! "},
	{"reflecting",	1.0,	0,	(BOLT_BOUNCES | BOLT_PREVENT_UNUSUAL | BOLT_PREVENT_TUNNELING), (BOLT_SELECTIVE | BOLT_PRECISION | BOLT_BOUNCES),
		"The reflecting rune makes bolts from $WANDORSTAFF continue through monsters, reflecting off walls if it has struck less than two creatures in its path. "},
	{"penetrating",	1.0,	0,	(BOLT_PENETRATING | BOLT_PREVENT_ALL_BUT_ELEMENTAL), (BOLT_PENETRATING),
		"The penetrating rune makes bolts from $WANDORSTAFF directly damage monsters; if any damage remains after killing the first monster in the path, it will continue onwards to the next. "},
	{"exploding",	1.0,	0,	(BOLT_EXPLODES | BOLT_PREVENT_ALL_BUT_ELEMENTAL), (BOLT_EXPLODES),					// exploding has a different effect for each staff, some non-obvious
		"The exploding rune makes bolts from $WANDORSTAFF explode when they hit their first target. "},
};

itemTable scrollTable[NUMBER_SCROLL_KINDS] = {
	{"enchanting",			itemTitles[0], "",	0,	550,	0,{0,0,0}, false, false, "This indispensable $SCROLLORTOME will imbue a single item with a powerful and permanent magical charge. A staff or charm will increase in power and charging speed; a staff will hold more charges; a weapon will inflict more damage or find its mark more frequently; a suit of armor will deflect additional blows; the effect of a ring on its wearer will intensify; and a wand will gain a single expendable charge. Weapons and armor will also require less strength to use, and any curses on the item will be lifted."}, // frequency is dynamically adjusted
	{"identify",			itemTitles[1], "",	30,	300,	0,{0,0,0}, false, false, "The scrying magic on $PARCHMENTORPAGES will permanently reveals all of the secrets of a single item."},
	{"teleportation",		itemTitles[2], "",	10,	500,	0,{0,0,0}, false, false, "The spell on $PARCHMENTORPAGES instantly transports the reader to a random location on the dungeon level. It can be used to escape a dangerous situation, but the unlucky reader might find himself in an even more dangerous place."},
	{"recharging",			itemTitles[3], "",	12,	375,	0,{0,0,0}, false, false, "The raw magical power bound up in $PARCHMENTORPAGES will, when released, recharge all of the reader's staffs and charms to full power and will add one or more charges to each wand."},
	{"protect armor",		itemTitles[4], "",	10,	400,	0,{0,0,0}, false, false, "The armor worn by the reader of this $SCROLLORTOME will be permanently proofed against degradation from acid."},
	{"protect weapon",		itemTitles[5], "",	10,	400,	0,{0,0,0}, false, false, "The weapon held by the reader of this $SCROLLORTOME will be permanently proofed against degradation from acid."},
	{"magic mapping",		itemTitles[6], "",	12,	500,	0,{0,0,0}, false, false, "When this is read, a purple-hued image of crystal clarity will be etched into your memory, alerting you to the precise layout of the level and revealing all hidden secrets. The locations of items and creatures will remain unknown."},
	{"cause fear",			itemTitles[7], "",	8,	500,	0,{0,0,0}, false, false, "A flash of red light will overwhelm all creatures in your field of view with terror, and they will turn and flee. Attacking a fleeing enemy will dispel the effect, and even fleeing creatures will turn to fight when they are cornered. Any allies caught within its blast will return to your side after the effect wears off, provided that you do not attack them in the interim."},
	{"negation",			itemTitles[8], "",	14,	400,	0,{0,0,0}, false, false, "This $SCROLLORTOME contains a powerful anti-magic. When it is released, all creatures (including yourself) and all items lying on the ground within your field of view will be exposed to its blast and stripped of magic -- and creatures animated purely by magic will die. Potions, scrolls, items being held by other creatures and items in your inventory will not be affected, but curses will be removed."},
	{"shattering",			itemTitles[9], "",	8,	500,	0,{0,0,0}, false, false, "The blast of sorcery unleashed by this $SCROLLORTOME will alter the physical structure of nearby stone, causing it to dissolve away over the ensuing minutes."},
	{"duplication",			itemTitles[10],"",	4,	850,	0,{0,0,0}, false, false, "This $SCROLLORTOME contains an intricate set of formulae which allows you to create anew any one item you already have in your possession. You will end up with multiple copies of darts and javelins and most potions and scrolls if you choose to duplicate them; save for scrolls of Enchanting and Duplication and potions of Strength and Life. Anything you have enchanted does not get the additional bonuses you gave it, but similarly acid damage is not copied to the new item. "},
	{"aggravate monsters",	itemTitles[11], "",	10,	50,		0,{0,0,0}, false, false, "When read aloud, this $SCROLLORTOME will unleash a piercing shriek that will awaken all monsters and alert them to the reader's location. Any monster in your line of sight will be temporarily aggravated, preferring to attack instead of fleeing."},
	{"summon monsters",		itemTitles[12], "",	10,	50,		0,{0,0,0}, false, false, "The incantation on this $SCROLLORTOME will call out to creatures in other planes of existence, drawing them through the fabric of reality to confront the reader."},
};

itemTable tomeTable[NUMBER_TOME_KINDS] = {
	{"",		itemCovers[0], "",	10,	1100,	0,{0,0,0}, false, false, "This eldritch edition will enchant every item in your inventory and on the level of the same classification as the item you choose to first enchant."},
	{"",		itemCovers[1], "",	10,	600,	0,{0,0,0}, false, false, "This cryptic catalog will reveal the identity of every item of the same class as the item you choose to identify; choosing an armor or weapon will divulge the enchantment and runics of all such items in your inventory and on the level."},
	{"",		itemCovers[2], "",	10,	1000,	0,{0,0,0}, false, false, "This marvellous manuscript instantly transports the reader to any location you have seen or visited on this dungeon level."},
	{"",		itemCovers[3], "",	12,	750,	0,{0,0,0}, false, false, " This rejuvenating reader provides twice the charges that a scroll would -- this overcharging on staffs and charms lasts until the item is reduced to the normal number of charges it can hold."},
	{"immunity",itemCovers[4], "",	10,	800,	0,{0,0,0}, false, false, "The armor worn by the reader of this leather-bound ledger will be permanently proofed against one random race of creature found in the underworld. Armours with runic effects already will merely be proofed against degradation from acid."},
	{"slaying",	itemCovers[5], "",	10,	800,	0,{0,0,0}, false, false, "The weapon held by the reader of this vorpal volume will be permanently rendered the bane of one random race of creature found in the underworld. Weapons with runic effects already will merely be proofed against degradation from acid."},
	{"",		itemCovers[6], "",	12,	1000,	0,{0,0,0}, false, false, " This amazing atlas will outline a similar image for every previous level of the dungeon, although secrets and undiscovered traps will remain hidden on these levels."},
	{"",		itemCovers[7], "",	8,	1000,	0,{0,0,0}, false, false, "The nightmarish visage on this cult compendium will terrify all your enemies on this dungeon level when you open it and read the incantations left inside. You and your allies will be left unaffected... for the moment."},
	{"",		itemCovers[8], "",	8,	800,	0,{0,0,0}, false, false, "This dull dictionary drains the abilities and temporary powers of all your enemies on the level, and strips the negative enchantments of any cursed item carried by a creature (including yourself) or found on the floor."},
	{"",		itemCovers[9],"",	8,	1000,	0,{0,0,0}, false, false, " This tome shatters twice the radius of a scroll."},
	{"triplication",itemCovers[10],"",4,1700,	0,{0,0,0}, false, false, " This tome produces twice as much as a scroll of duplication."},
	{"",		itemCovers[11], "",	10,	500,	0,{0,0,0}, false, false, "This brutal book will drive all your enemies on the level into a fit of madness, briefly confusing them and forcing them to close and attack instead of fleeing or keeping their distance."},
	{"",		itemCovers[12], "",	10,	1000,	0,{0,0,0}, false, false, " This transdimensional tome then binds one of the summoned monsters to your service."},
};

itemTable potionTable[NUMBER_POTION_KINDS] = {
	{"life",				itemColors[0], "",	0,	500,	0,{0,0,0}, false, false, "A swirling tonic that will instantly heal you, cure you of ailments, and permanently increase your maximum health."}, // frequency is dynamically adjusted
	{"strength",			itemColors[1], "",	0,	400,	0,{0,0,0}, false, false, "This powerful medicine will course through your muscles, permanently increasing your strength by one point."}, // frequency is dynamically adjusted
	{"telepathy",			itemColors[2], "",	16,	350,	0,{0,0,0}, false, false, "After drinking this, your mind will become attuned to the psychic signature of distant creatures, enabling you to sense biological presences through walls. Its effects will not reveal inanimate objects, such as totems, turrets and traps."},
	{"levitation",			itemColors[3], "",	15,	250,	0,{0,0,0}, false, false, "Drinking this curious liquid will cause you to hover five to ten feet in the air, able to drift effortlessly over lava, water, chasms and traps. Flames, gases and spiderwebs fill the air, however, and cannot be bypassed while airborne. Creatures that dwell in water or mud will be unable to attack you while you levitate."},
	{"detect magic",		itemColors[4], "",	20,	300,	0,{0,0,0}, false, false, "This drink will sensitize your mind to the radiance of magic. Items imbued with helpful enchantments will be marked on the map with a full magical sigil; items corrupted by curses or intended to inflict harm will be marked on the map with a hollow sigil. The Amulet of Yendor, if in the vicinity, will be revealed by its unique aura."},
	{"speed",				itemColors[5], "",	10,	500,	0,{0,0,0}, false, false, "Quaffing the contents of this flask will enable you to move at blinding speed for several minutes."},
	{"fire immunity",		itemColors[6], "",	15,	500,	0,{0,0,0}, false, false, "This $POTIONORELIXIR will render you impervious to heat and permit you to wander through fire and lava and ignore otherwise deadly bolts of flame. It will not guard against the concussive impact of an explosion, however."},
	{"invisibility",		itemColors[7], "",	15,	400,	0,{0,0,0}, false, false, "Drinking this $POTIONORELIXIR will render you temporarily invisible. While invisible, enemies will track you only with great difficulty, and will be completely unable to detect you from more than two meters away."},
	{"winds",				itemColors[8], "",	7,	450,	0,{0,0,0}, false, false, "The bottled winds stored in this flask swirl around, dispersing any gases in the air. If you simply opened the flask rather than breaking it, the winds are then sucked back into the bottle, snapping the lid shut. If sufficient gas has been captured in this process, you will end up with another $POTIONORELIXIR type."},
	{"poisonous gas",		itemColors[9], "",	10,	200,	0,{0,0,0}, false, false, "Uncorking or shattering this pressurized glass will cause its contents to explode into a deadly cloud of caustic purple gas. You might choose to fling this $POTIONORELIXIR at distant enemies instead of uncorking it by hand."},
	{"paralysis",			itemColors[10], "",	10, 250,	0,{0,0,0}, false, false, "Upon exposure to open air, the liquid in this flask will vaporize into a numbing pink haze. Anyone who inhales the cloud will be paralyzed instantly, unable to move for some time after the cloud dissipates. This item can be thrown at distant enemies to catch them within the effect of the gas."},
	{"hallucination",		itemColors[11], "",	10,	500,	0,{0,0,0}, false, false, "This flask contains a vicious and long-lasting hallucinogen. Under its dazzling effect, you will wander through a rainbow wonderland, unable to discern the form of any creatures or items you see."},
	{"confusion",			itemColors[12], "",	10,	450,	0,{0,0,0}, false, false, "This unstable chemical will quickly vaporize into a glittering cloud upon contact with open air, causing any creature that inhales it to lose control of the direction of its movements until the effect wears off (although its ability to aim projectile attacks will not be affected). Its vertiginous intoxication can cause creatures and adventurers to careen into one another or into chasms or lava pits, so extreme care should be taken when under its effect. Its contents can be weaponized by throwing the flask at distant enemies."},
	{"incineration",		itemColors[13], "",	15,	500,	0,{0,0,0}, false, false, "This flask contains an unstable compound which will burst violently into flame upon exposure to open air. You might throw the flask at distant enemies -- or into a deep lake, to cleanse the cavern with scalding steam."},
	{"darkness",			itemColors[14], "",	7,	150,	0,{0,0,0}, false, false, "Drinking this $POTIONORELIXIR will plunge you into darkness. At first, you will be completely blind to anything not illuminated by an independent light source, but over time your vision will regain its former strength. Throwing the $POTIONORELIXIR will create a cloud of supernatural darkness, and enemies will have difficulty seeing or following you if you take refuge under its cover."},
	{"descent",				itemColors[15], "",	15,	500,	0,{0,0,0}, false, false, "When this flask is uncorked by hand or shattered by being thrown, the fog that seeps out will temporarily cause the ground in the vicinity to vanish."},
	{"creeping death",		itemColors[16], "",	7,	450,	0,{0,0,0}, false, false, "When the cork is popped or the flask is thrown, tiny spores will spill across the ground and begin to grow a deadly lichen. Anything that touches the lichen will be poisoned by its clinging tendrils, and the lichen will slowly grow to fill the area. Fire will purge the infestation."},
	{"water",				itemColors[17], "",	7,	450,	0,{0,0,0}, false, false, "Releasing the contents of this flask floods the surrounding area with deep water; which explodes into steam if it comes into contact with lava."},
	{"stench",				itemColors[18], "",	0,	450,	0,{0,0,0}, false, false, "The foul and malodorous vapours contained in this flask force everyone around them to gag and vomit uncontrollably."},
	{"explosive gas",		itemColors[19], "",	0,	450,	0,{0,0,0}, false, false, "If thrown, the pressurized gases stored in this flask explode violently, with enough power to topple nearby statues."},
};

itemTable elixirTable[NUMBER_ELIXIR_KINDS] = {
	{"",		itemChemistry[0], "",	0,	1000,	0,{0,0,0}, false, false, " This elixir temporarily protects you from harm."},
	{"",		itemChemistry[1], "",	0,	800,	0,{0,0,0}, false, false, "This powerful medicine will course through your muscles, permanently increasing your strength by two points and providing a further temporary boost for some time."},
	{"",		itemChemistry[2], "",	16,	700,	0,{0,0,0}, false, false, " A psychic link will be remain between you and any creatures on the level when you first use this potion, including your allies."},
	{"",		itemChemistry[3], "",	15,	500,	0,{0,0,0}, false, false, " This elixir cannot be negated."},
	{"",		itemChemistry[4], "",	20,	600,	0,{0,0,0}, false, false, " You will learn the maximum number of charges on any wand or staff you detect and weapons and armor will be identified and have the presence of runes revealed."},
	{"",		itemChemistry[5], "",	10,	1000,	0,{0,0,0}, false, false, " This elixir makes you move twice as quickly again and cannot be negated."},
	{"",		itemChemistry[6], "",	15,	1000,	0,{0,0,0}, false, false, " This elixir also makes you immune to the explosive force of marsh gas and cannot be negated."},
	{"",		itemChemistry[7], "",	15,	800,	0,{0,0,0}, false, false, " This elixir prevents even monsters adjacent to you from seeing you."},
	{"",		itemChemistry[8], "",	7,	900,	0,{0,0,0}, false, false, " Throwing this elixir creates a vent which fills periodically with wind."},
	{"",		itemChemistry[9], "",	0,	400,	0,{0,0,0}, false, false, " Rapidly gulping down part of this elixir will make you temporarily immune to poisons even as it fills the air around you; the shattered remains of the flask when thrown become a trap for those who step on it."},
	{"",		itemChemistry[10], "",	0,	500,	0,{0,0,0}, false, false, " Quickly swallowing a fraction of this elixir will preven you being paralyzed even as it fills the air around you; the shattered remains of the flask when thrown become a trap for those who step on it."},
	{"",		itemChemistry[11], "",	0,	1000,	0,{0,0,0}, false, false, " The effects of this elixir turn the world into a dream around you, which you wake from instead of dying."},
	{"",		itemChemistry[12], "",	0,	900,	0,{0,0,0}, false, false, " Briefly sipping this elixir will make you temporarily immune to confusion even as it fills the air around you; the shattered remains of the flask when thrown become a trap for those who step on it."},
	{"",		itemChemistry[13], "",	0,	1000,	0,{0,0,0}, false, false, " Burning your lip on the brim of this elixir will make you temporarily immune to fire even as it fills the air around you; the shattered remains of the flask when thrown become a trap for those who step on it."},
	{"light",	itemChemistry[14], "",	0,	300,	0,{0,0,0}, false, false, "Drinking this elixir will intensify the light around you, letting you see further in the gloom; throwing it fills the dungeon level with a large pool of radiance."},
	{"",		itemChemistry[15], "",	0,	1000,	0,{0,0,0}, false, false, " Sinking a draught of this elixir will let you float above the pit it creates; if thrown the centre of the pit will remain even after the rest vanishes."},
	{"",		itemChemistry[16], "",	0,	900,	0,{0,0,0}, false, false, " Choking down the spores of this elixir will make you temporarily immune to poison even as it spills to the ground around you; a weird plant will emerge from the shattered remains of this flask when thrown."},
	{"holy water",itemChemistry[17], "",0,	900,	0,{0,0,0}, false, false, " Quenching your thirst with this elixir will temporarily bless you, halving the damage you take and letting you walk on water even as it floods the dungeon around you; the shattered remains of the flask when thrown become a trap for those who step on it."},
	{"",		itemChemistry[18], "",	0,	900,	0,{0,0,0}, false, false, " Gagging on this vile elixir will make you temporarily immune to poison even as it fills the air around you; a foul plant takes root in the shattered remains of this flask when thrown."},
	{"",		itemChemistry[19], "",	0,	900,	0,{0,0,0}, false, false, " Inhaling this elixir will make you temporarily immune to explosions even as it fills the air around you; the shattered remains of the flask when thrown continue to hiss and spark into flame."},
};

itemTable boltTable[NUMBER_BOLT_KINDS] = {
	{"force",			"", "",	8,	500,	0,{3,5,1}, false, false, "The blast from this $WANDORSTAFF propels the first creature it hits backwards at high speed. It inflicts triple damage if the creature is thrown against a wall or can be used to push the target into chasms, deep water or deadly lava."},
	{"lightning",		"", "",	10,	1300,	0,{5,8,1}, false, false, "This $WANDORSTAFF conjures forth deadly arcs of electricity, which deal damage to any number of creatures in a straight line. It will reflect off walls if it has struck less than two creatures in its path and stop in a shower of sparks if it hits a target standing in water."},
	{"firebolt",		"", "",	15,	1300,	0,{4,6,1}, false, false, "This $WANDORSTAFF unleashes bursts of magical fire. It will ignite flammable terrain, and will damage and burn a creature that it hits. Creatures with an immunity to fire will be unaffected by the bolt."},
	{"poison",			"", "",	10,	1200,	0,{4,6,1}, false, false, "The vile blast of this twisted bit of $WOODORMETAL will imbue its target with a deadly venom. A creature that is poisoned will suffer one point of damage per turn and will not regenerate lost health until the effect ends."},
	{"tunneling",		"", "",	10,	1000,	0,{3,5,1}, false, false, "Bursts of magic from this $WANDORSTAFF will pass harmlessly through creatures but will reduce walls and other inanimate obstructions to rubble."},
	{"detonation",		"", "",	4,	800,	0,{4,6,1}, false, false, "A bolt from this $WANDORSTAFF will halve the maximum health and defense of any creature it hits but primes the creature to explode when it dies."},
	{"blinking",		"", "",	11,	1200,	0,{5,8,1}, false, false, "This $WANDORSTAFF will allow you to teleport in the chosen direction. Creatures and inanimate obstructions will block the teleportation. Be careful around dangerous terrain, as nothing will prevent you from teleporting to a fiery death in a lake of lava."},
	{"conjuration",		"", "",	8,	1000,	0,{4,6,1}, false, false, "A flick of this $WANDORSTAFF summons a number of phantom blades to fight on your behalf."},
	{"teleportation",	"", "",	5,	800,	0,{2,5,1}, false, false, "A blast from this $WANDORSTAFF will teleport a creature to a random place on the level. This can be particularly effective against aquatic or mud-bound creatures, which are helpless on dry land."},
	{"slowness",		"", "",	5,	800,	0,{2,5,1}, false, false, "This $WANDORSTAFF will cause a creature to move at half its ordinary speed for several turns."},
	{"negation",		"", "",	8,	550,	0,{4,6,1}, false, false, "This powerful anti-magic will strip a creature of a host of magical traits, including flight, invisibility, acidic corrosiveness, telepathy, magical speed or slowness, hypnosis, magical fear, immunity to physical attack, fire resistance and the ability to blink at will. Spell casters will lose their magical abilities and magical totems will be rendered inert. Creatures animated purely by magic will die."},
	{"domination",		"", "",	6,	1000,	0,{1,2,1}, false, false, ""},
	{"entrancement",	"", "",	5,	1000,	0,{4,6,1}, false, false, "This curious $WANDORSTAFF will send creatures into a deep but temporary trance, in which they will mindlessly mirror your movements. You can use the effect to cause one creature to attack another or to step into lava or other hazardous terrain."},
	{"obstruction",		"", "",	10,	1000,	0,{4,6,1}, false, false, "A mass of impenetrable green crystal will spring forth from the point at which this $WANDORSTAFF is aimed, obstructing any who wish to move through the affected area and temporarily entombing any who are already there. The crystal will dissolve into the air as time passes."},
	{"discord",			"", "",	10,	1000,	0,{4,6,1}, false, false, "The purple light from this $WANDORSTAFF will alter the perceptions of all creatures to think the target is their enemy. Strangers and allies alike will turn on an affected creature."},
	{"nature",			"", "",	4,	600,	0,{4,6,1}, false, false, "This length of twisted $WOODORMETAL has been attuned to nature to rapidly grow a useful plant if the bolt this $WANDORSTAFF fires passes over three grids of the same fertile soil. Observe the dungeon or experiment to see what types of plants you can grow."},
	{"plenty",			"", "",	4,	700,	0,{1,2,1}, false, false, "The creature at the other end of this mischievous bit of $WOODORMETAL will be beside itself -- literally! Cloning an enemy is ill-advised, but the effect can be invaluable on a powerful ally."},
	{"polymorphism",	"", "",	4,	700,	0,{3,5,1}, false, false, "This mischievous magic can transform any creature into another creature at random. Beware: the tamest of creatures might turn into the most fearsome."},
	{"beckoning",		"", "",	6,	500,	0,{2,4,1}, false, false, "The force of this $WANDORSTAFF will yank the targeted creature into direct proximity."},
	{"invisibility",	"", "",	4,	100,	0,{3,5,1}, false, false, "A charge from this $WANDORSTAFF will render a creature temporarily invisible to the naked eye. Only with telepathy or in the silhouette of a thick gas will an observer discern the creature's hazy outline."},
	{"healing",			"", "",	4,	1100,	0,{4,6,1}, false, false, "The crimson bolt from this $WANDORSTAFF will heal the injuries of any creature it touches. This can be counterproductive against enemies but can prove useful when aimed at your allies. Unfortunately, you cannot use this or any $WANDORSTAFF on yourself."},
	{"haste",			"", "",	6,	900,	0,{2,4,1}, false, false, "The magical bolt from this $WANDORSTAFF will temporarily double the speed of any monster it hits. This can be counterproductive against enemies but can prove useful when aimed at your allies. Unfortunately, you cannot use this or any $WANDORSTAFF on yourself."},
	{"protection",		"", "",	6,	900,	0,{2,4,1}, false, false, "A charge from this $WANDORSTAFF will bathe a creature in protective light, absorbing all damage until depleted. This can be counterproductive against enemies but can prove useful when aimed at your allies. Unfortunately, you cannot use this or any $WANDORSTAFF on yourself."},
	{"reflection",		"", "",	4,	1100,	0,{2,4,1}, false, false, "This $WANDORSTAFF wraps its target in a highly reflective barrier which deflects any magical bolt which strikes it in a random direction."},
	{"sentries",		"", "",	6,	900,	0,{3,5,1}, false, false, "This creates a turret at the terminus of the bolt which starts out attacking anything around it including you!"},
//	{"phantoms",		"", "",	2,	600,	0,{2,4,1}, false, false, "This summons a phantom at the terminus of the bolt which starts out attacking anything around it including you! This $WANDORSTAFF also has a chance of darkening everything around the summoned phantom."},
	{"zombies",			"", "",	2,	600,	0,{2,4,1}, false, false, "This summons a zombie at the terminus of the bolt which starts out attacking anything around it including you!"},
	{"bloats",			"", "",	2,	800,	0,{5,8,1}, false, false, ""},
	{"spiders",			"", "",	2,	600,	0,{2,4,1}, false, false, "This creates spider webs along the path of the bolt and summons a spider at the terminus which starts out attacking anything around it including you!"},
	{"nagas",			"", "",	2,	600,	0,{2,4,1}, false, false, "This creates a pool of water along the path of the bolt and summons a naga at the terminus which starts out attacking anything around it including you!"},
	{"pixies",			"", "",	2,	600,	0,{2,4,1}, false, false, "This summons a pixie at the terminus of the bolt which starts out attacking anything around it including you!"},
	{"toads",			"", "",	2,	600,	0,{2,4,1}, false, false, "This summons a toad at the terminus of the bolt which starts out attacking anything around it including you!"},
	{"underworms",		"", "",	2,	800,	0,{2,4,1}, false, false, "This summons an underworm at the terminus of the bolt which starts out attacking anything around it including you!"},
	{"mandrakes",		"", "",	1,	800,	0,{2,4,1}, false, false, "This plants a mandrake at the terminus of the bolt which quickly matures to fight along side you. The mandrake has only a limited lifespan."},
	{"dead man's ear",	"", "",	1,	800,	0,{2,4,1}, false, false, "This plants a dead man's ear at the terminus of the bolt. The dead man's ear has only limited lifespan."},
	{"crimson cap",		"", "",	1,	800,	0,{2,4,1}, false, false, "This plants a crimson cap at the terminus of the bolt. The crimson cap has only a limited lifespan. The bolt will pass through one creature or wall grid unless there is no space on the other side but stops normally after that."},
	{"mirrored totem",	"", "",	4,	800,	0,{2,4,1}, false, false, "This creates a mirrored totem at the terminus of the bolt, surrounded by glyphs. You need to create two mirrored totems in line of sight of each other for the glyphs to function -- and the totems created by this $WANDORSTAFF does not work with existing glyphs found in the dungeon."},
};

// wand frequencies are hardcoded in roguemain.c and overwrite values given here
itemTable wandTable[NUMBER_WAND_KINDS] = {
	{"",	itemMetals[0], "",	10,	500,	0,{3,5,1}, false, false, ""},
	{"",	itemMetals[1], "",	10,	1300,	0,{5,8,1}, false, false, ""},
	{"",	itemMetals[2], "",	10,	1300,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[3], "",	10,	1200,	0,{4,6,1}, false, false, " The poisons emitted by this wand can fell even a deadly dragon with a single use -- eventually."},
	{"",	itemMetals[4], "",	10,	1000,	0,{3,5,1}, false, false, ""},
	{"",	itemMetals[5],	"", 10,	800,	0,{4,6,1}, false, false, "This wand starts a short count down timer which ends in the target's spontaneous detonation -- crippling it in the interim by halving the creature's maximum health and defense."},
	{"",	itemMetals[6], "",	10,	1200,	0,{5,8,1}, false, false, ""},
	{"",	itemMetals[7], "",	10,	1000,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[8], "",	10,	800,	0,{2,5,1}, false, false, ""},
	{"",	itemMetals[9], "",	10,	800,	0,{2,5,1}, false, false, ""},
	{"",	itemMetals[10], "",	10,	550,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[11], "",	10,	1000,	0,{1,2,1}, false, false, "This wand can forever bind an enemy to the caster's will, turning it into a steadfast ally. However, the magic works only against enemies that are near death."},
	{"",	itemMetals[12], "",	10,	1000,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[13], "",	10,	1000,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[14], "",	10,	1000,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[15], "",	10,	500,	0,{4,6,1}, false, false, " This wand will also grow wooden bridges over chasms."},
	{"",	itemMetals[16], "",	10,	700,	0,{1,2,1}, false, false, ""},
	{"",	itemMetals[17], "",	10,	700,	0,{3,5,1}, false, false, " The horror of permanent transformation will turn any affected allies against you."},
	{"",	itemMetals[18], "",	10,	500,	0,{2,4,1}, false, false, ""},
	{"",	itemMetals[19], "",	10,	100,	0,{3,5,1}, false, false, ""},
	{"",	itemMetals[20], "",	10,	1100,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[21], "",	10,	900,	0,{2,4,1}, false, false, ""},
	{"",	itemMetals[22], "",	10,	900,	0,{2,4,1}, false, false, ""},
	{"",	itemMetals[23], "",	10,	1100,	0,{2,4,1}, false, false, ""},
	{"",	itemMetals[24], "",	5,	900,	0,{3,5,1}, false, false, " Turrets summoned this way become friendly towards you after some time."},
//	{"phantoms",		itemMetals[24], "",	1,	600,	0,{2,4,1}, false, false, " Phantoms summoned this way become friendly towards you after some time."},
	{"",	itemMetals[25], "",	1,	600,	0,{2,4,1}, false, false, " Zombies summoned this way become friendly towards you after some time."},
	{"",	itemMetals[26], "",	2,	800,	0,{5,8,1}, false, false, "This summons one or more bloats at the terminus of the bolt which start out attacking anything around them including you!"},
	{"",	itemMetals[27], "",	1,	600,	0,{2,4,1}, false, false, " Spiders summoned this way become friendly towards you after some time."},
	{"",	itemMetals[28], "",	1,	600,	0,{2,4,1}, false, false, " Nagas summoned this way become friendly towards you after some time."},
	{"",	itemMetals[29], "",	1,	600,	0,{2,4,1}, false, false, " Pixies summoned this way become friendly towards you after some time."},
	{"",	itemMetals[30], "",	1,	600,	0,{2,4,1}, false, false, " Toads summoned this way become friendly towards you after some time."},
	{"",	itemMetals[31], "",	1,	800,	0,{2,4,1}, false, false, " Underworms summoned this way become friendly towards you after some time."},
	{"",	itemMetals[32], "",	1,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemMetals[33], "",	1,	800,	0,{4,5,1}, false, false, ""},
	{"",	itemMetals[34], "",	1,	800,	0,{4,6,1}, false, false, ""},
	{"",	itemMetals[35], "",	1,	800,	0,{4,6,1}, false, false, ""},
};

itemTable staffTable[NUMBER_STAFF_KINDS] = {
	{"",	itemWoods[0], "",	8,	500,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[1], "",	10,	1300,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[2], "",	15,	1300,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[3], "",	10,	1200,	0,{2,4,1}, false, false, " The duration of the effect increases exponentially with the level of the staff, and a level 10 staff can fell even a deadly dragon with a single use -- eventually."},
	{"",	itemWoods[4], "",	10,	1000,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[5], "",	4,	1000,	0,{2,4,1}, false, false, "A bolt from this staff will halve the maximum health and defense of any creature it hits but primes the creature to explode when it dies -- an event which the staff will precipitate by shortening the target's lifespan."},
	{"",	itemWoods[6], "",	11,	1200,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[7], "",	8,	1000,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[8], "",	5,	800,	0,{2,4,1}, false, false, " The enchantment level of the staff determines the maximum range of the teleport."},
	{"",	itemWoods[9], "",	5,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[10], "",	8,	550,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[11], "",	6,	1000,	0,{2,4,1}, false, false, "This staff temporarily binds an enemy to the caster's will, turning it into a steadfast ally for the duration of its effect, and removes discord from your allies. Enemies which are hunting you or fleeing are unaffected unless they are near death."},
	{"",	itemWoods[12], "",	5,	1000,	0,{2,4,1}, false, false, " The spell will be broken if you attack the creature under the effect."},
	{"",	itemWoods[13], "",	10,	1000,	0,{2,4,1}, false, false, " Higher level staffs will create larger obstructions."},
	{"",	itemWoods[14], "",	10,	1000,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[15], "",	4,	500,	0,{2,4,1}, false, false, " This staff cannot grow these plants too closely together and witch hazel completely discharges it, recharging all your other staffs by the number of charges lost."},
	{"",	itemWoods[16], "",	4,	700,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[17], "",	4,	700,	0,{2,4,1}, false, false, " The transformation is only temporary unless the affected target is killed in the interim."},
	{"",	itemWoods[18], "",	6,	500,	0,{2,4,1}, false, false, " The enchantment level of the staff determines the maximum range of the beckoning."},
	{"",	itemWoods[19], "",	4,	100,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[20], "",	4,	1100,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[21], "",	6,	900,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[22], "",	6,	900,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[23], "",	4,	1100,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[24], "",	6,	900,	0,{2,4,1}, false, false, ""},
//	{"phantoms",		itemWoods[24], "",	2,	600,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[25], "",	2,	600,	0,{2,4,1}, false, false, " This staff also has a chance of setting the zombie afire."},
	{"",	itemWoods[26], "",	2,	800,	0,{2,4,1}, false, false, "This summons one or more bloats at the terminus of the bolt which are hostile towards you."},
	{"",	itemWoods[27], "",	2,	600,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[28], "",	2,	600,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[29], "",	2,	600,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[30], "",	2,	600,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[31], "",	2,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[32], "",	2,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[33], "",	2,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[34], "",	2,	800,	0,{2,4,1}, false, false, ""},
	{"",	itemWoods[35], "",	2,	800,	0,{2,4,1}, false, false, ""},
};

itemTable summoningStaffTable[1] = {
	{"summoning",		itemWoods[14], "",	0,	  0,	0,{0,0,0}, false, false, "This summons a monster at the terminus of the bolt which starts attacking anything around it, but eventually becomes hostile towards you."},
};

itemTable ringTable[NUMBER_RING_KINDS] = {
	{"clairvoyance",	itemGems[0], "",	1,	900,	0,{1,3,1}, false, false, "Wearing this ring will permit you to see through nearby walls and doors, within a radius determined by the level of the ring. A cursed ring of clairvoyance will blind you to your immediate surroundings."},
	{"stealth",			itemGems[1], "",	1,	800,	0,{1,3,1}, false, false, "Enemies will be less likely to notice you if you wear this ring. Staying motionless and lurking in the shadows will make you even harder to spot. At very high levels, even enemies giving chase may sometimes lose track of you. Cursed rings of stealth will alert enemies who might otherwise not have noticed your presence."},
	{"regeneration",	itemGems[2], "",	1,	750,	0,{1,3,1}, false, false, "This ring increases the body's regenerative properties, allowing one to recover lost health at an accelerated rate. Cursed rings will decrease or even halt one's natural regeneration."},
	{"transference",	itemGems[3], "",	1,	750,	0,{1,3,1}, false, false, "Landing a melee attack while wearing this ring will cause a proportion of the inflicted damage to transfer to benefit of the attacker's own health. Cursed rings will cause you to lose health with each attack you land."},
	{"light",			itemGems[4], "",	1,	600,	0,{1,3,1}, false, false, "This ring subtly enhances your vision, enabling you to see farther in the dimming light of the deeper dungeon levels. It will not make you more visible to enemies."},
	{"awareness",		itemGems[5], "",	1,	700,	0,{1,3,1}, false, false, "Wearing this ring will allow the wearer to notice hidden secrets -- traps and secret doors -- without taking time to search. Cursed rings of awareness will dull your senses, making it harder to notice secrets even when actively searching for them."},
	{"wisdom",			itemGems[6], "",	1,	700,	0,{1,3,1}, false, false, "Your staffs will recharge at an accelerated rate in the energy field that radiates from this ring. Cursed rings of wisdom will instead cause your staffs to recharge more slowly."},
	{"might",			itemGems[7], "",	1,	700,	0,{1,3,1}, false, false, "This ring simply increases your strength, or if cursed, decreases it."},
	{"telepathy",		itemGems[8], "",	1,	800,	0,{1,3,1}, false, false, "This ring attunes you to minds nearby, allowing you to look through their eyes."},
};

itemTable charmTable[NUMBER_CHARM_KINDS] = {
	{"health",          "", "",	1,	900,	0,{1,3,1}, true, false, "This handful of dried bloodwort and mandrake root has been bound together with leather cord and imbued with a powerful healing magic."},
	{"strength",		"", "",	0,	400,	0,{1,3,1}, true, false, "A black doctors bag containing small vials of a serum of centipede venom."},
	{"telepathy",		"", "",	1,	700,	0,{1,3,1}, true, false, "Seven tiny glass eyes roll freely within this glass sphere. Somehow, they always come to rest facing outward."},
	{"levitation",      "", "",	1,	700,	0,{1,3,1}, true, false, "Sparkling dust and fragments of feather waft and swirl endlessly inside this small glass sphere."},
	{"detect magic",	"", "",	0,	300,	0,{1,3,1}, true, false, "A kind of mechanical dowsing device, with a brass needle which spins uncontrollably."},
	{"haste",           "", "",	1,	750,	0,{1,3,1}, true, false, "Various animals have been etched into the surface of this brass bangle. It emits a barely audible hum."},
	{"fire immunity",	"", "",	1,	750,	0,{1,3,1}, true, false, "Eldritch flames flicker within this polished crystal bauble."},
	{"invisibility",	"", "",	1,	700,	0,{1,3,1}, true, false, "This intricate figurine depicts a strange humanoid creature. It has a face on both sides of its head, but all four eyes are closed."},
	{"winds",			"", "",	0,	900,	0,{1,3,1}, true, false, "A set of hand-pumped bellows which plays a wheezy tune when you compress it."},
	{"poisonous gas",	"", "",	0,	400,	0,{1,3,1}, true, false, "This is the shrivelled sac from a long dead bloat, still warm and soft to the touch."},
	{"paralysis",		"", "",	0,	500,	0,{1,3,1}, true, false, "Amber beads strung together, each with an insect trapped inside."},
	{"discord",			"", "",	0,	1000,	0,{1,3,1}, true, false, "A suggestion box filled with a clamour of voices calling for contradictory requests: the first note you read asks for every item to be useful."},
	{"confusion",		"", "",	0,	900,	0,{1,3,1}, true, false, "An ornate music box: when you wind it only random noise comes out."},
	{"incineration",	"", "",	0,	1000,	0,{1,3,1}, true, false, "Fire and ash and soot and flame and burning and light and heat and warmth and hunger and flesh and want and feed.."},
	{"darkness",		"", "",	0,	300,	0,{1,3,1}, true, false, "No matter how hard you look, this strange object is never quite there."},
	{"descent",			"", "",	0,	1000,	0,{1,3,1}, true, false, "Someone has stitched this ugly balloon from the skin of pit bloat."},
	{"creeping death",	"", "",	0,	900,	0,{1,3,1}, true, false, "A fungal pod which glows with a sickly luminescence and grafts itself to your flesh as you hold it. When you pull it away, it peels off the outer layer of your skin."},
	{"water",			"", "",	0,	900,	0,{1,3,1}, true, false, "This silver horn is embossed with sea-scenes, sirens and the sound of shanties."},
	{"stench",			"", "",	0,	900,	0,{1,3,1}, true, false, "Flies buzz around this encrusted pouch. Ugh. It is better not describing the contents..."},
	{"explosive gas",	"", "",	0,	900,	0,{1,3,1}, true, false, "A sealed tin can painted a drab green with a wooden handle to assist throwing. Numbers have been stencilled on it with a faded yellow paint."},
	{"charming",		"", "",	0,	1000,	0,{1,3,1}, true, false, "A silver charm bracelet: this is a bloat, this is a rat, this one here is a vampire bat..."},
	{"identify",		"", "",	0,	300,	0,{1,3,1}, true, false, "Who doesn't want such a pair of such smart-looking spectacles? You feel ever so clever to have made them..."},
	{"teleportation",   "", "",	1,	700,	0,{1,3,1}, true, false, "The surface of this nickel sphere has been etched with a perfect grid pattern. Somehow, the squares of the grid are all exactly the same size."},
	{"recharging",      "", "",	1,	700,	0,{1,3,1}, true, false, "A strip of bronze has been wound around a rough wooden sphere. Each time you touch it, you feel a tiny electric shock."},
	{"protection",		"", "",	1,	800,	0,{1,3,1}, true, false, "Four copper rings have been joined into a tetrahedron. The construct is oddly warm to the touch."},
	{"protection",		"", "",	0,	800,	0,{1,3,1}, true, false, "Four copper rings have been joined into a tetrahedron. The construct is oddly warm to the touch."},
	{"magic mapping",	"", "",	0,	500,	0,{1,3,1}, true, false, "A well worn map folded in the shape of a flower. Every time you unfold it, the contours of its contents have changed."},
	{"fear",            "", "",	1,	700,	0,{1,3,1}, true, false, "When you gaze into the murky interior of this obsidian cube, you feel as though something predatory is watching you."},
	{"negation",        "", "",	1,	700,	0,{1,3,1}, true, false, "A featureless gray disc hangs from a fine silver chain. When you touch it, your hand briefly goes numb."},
	{"shattering",      "", "",	1,	700,	0,{1,3,1}, true, false, "This turquoise crystal, fixed to a leather lanyard, hums with arcane energy."},
	{"duplication",		"","",	0,	850,	0,{1,3,1}, true, false, "This macabre puzzle box can only bring bad things into the world."},
	{"aggravate monsters","", "",0,	50,		0,{1,3,1}, true, false, "Shaking this rattle sends echos around the dungeon, cascading in the deep to become as loud as war drums."},
	{"summon monsters",	"", "",	0,	50,		0,{1,3,1}, true, false, "A spherical device consisting of bands of metal which have constricted themselves tightly together. Is there something trapped inside?"},
};

itemTable talismanTable[NUMBER_TALISMAN_KINDS] = {
	{"madness",				itemMaterials[0], "",	1,	500,	0,{2,4,1}, false, false, "This dazzling emblem causes your hallucinations to become infectious, spreading discordance to any creature you strike and boosting the enchantment of your weapons and armor while you are under the influence of a hallucinogen; or merely increases the length you suffer such a malady if cursed."},
	{"spiders",				itemMaterials[1], "",	1,	600,	0,{2,4,1}, false, false, "This grisly decoration lets you move freely through spider webs, and makes your weapons transfer life to you while you are poisoned; or merely increases the duration that poison affects you for if cursed. Enchanting it will summon a spider as a companion from another level and the enchantment level will determine the power of the summoned spider."},
	{"flame dancing",		itemMaterials[2], "",	1,	800,	0,{2,4,1}, false, false, "This bright crest boosts the enchantment of the rings you are wearing while you are on fire; or merely increases the time you burn for if cursed. Wearing this talisman delays damage from burning until after your fire immunity expires."},
//	{"the jackal",			itemMaterials[3], "",	0,	900,	0,{2,4,1}, false, false, "This jagged mouthpiece lets you eat the dead to completely heal yourself and summon a pack of jackals to accompany you. You enchant this to make replacement jackals stronger."},
//	{"trap mastery",		itemMaterials[4], "",	0,	900,	0,{2,4,1}, false, false, "This whirring mechanism lets you safely disarm a trap by moving into the same grid occupied by the trigger; this will also recover the potion used to create it. You may also set traps by dropping a potion of poisonous gas, paralysis, confusion, incineration or water -- this will take two extra actions to complete so be cautious doing this near the enemy."},
//	{"chaos",				itemMaterials[5], "",	0,	800,	0,{0,0,0}, false, false, "This warped icon leaves you at the whim of the gods of Chaos every time you enchant it. If you acquire powers which must be aimed at a target, you should apply this talisman while it is equipped to use one at random."},
	{"sacrifice",			itemMaterials[6], "",	1,	700,	0,{0,0,0}, false, false, "This impassive idol blesses you every time an item burns up in the dungeon. You take half damage and can walk on water while blessed; removing the idol removes any blessing it has granted."},
	{"wizardry",			itemMaterials[7], "",	1,	1000,	0,{0,0,0}, false, false, "This wondrous widget unlocks a hidden runic power in every wand and staff you carry, as long as you wear this talisman."},
	{"balance",				itemMaterials[8], "",	1,	500,	0,{0,0,0}, false, false, "This discarded diadem allows you to use the negative energies of wearing a cursed ring to boost the enchantment of a beneficial ring on the other hand by double the negative enchantment. You can enchant this to worsen the enchantment of any cursed ring you are wearing."},
	{"dungeoneering",		itemMaterials[9], "",	1,	1200,	0,{0,0,0}, false, false, "This useful tchotchke heals and protects you as you discover new areas in the dungeon; but you incur a debt which prevents you from healing naturally both while you wear and once you remove the talisman. It appears to have been gathering dust atop a desk for a few years."},
	{"the rune smiths",		itemMaterials[10], "",	1,	1300,	0,{0,0,0}, false, false, "This eldritch lens has a chance of imbuing a rune any time you enchant a weapon or armour without one; identifying the weapon or armour will tell you how many enchantments this will take. Runes revealed this way have no effect unless you are wearing this talisman and more importantly, enchanting an item with a rune will have no effect except to reduce its strength requirement if you are not strong enough to use it."},
	{"shape changing",		itemMaterials[11], "",	0,	1100,	0,{0,0,0}, false, false, "This formless mask lets you consume fallen monsters in order to assume their shape -- but take care to never eat a monster while you are already transformed. You cannot wear armour or use weapons while in another shape; but you should apply this talisman while it is equipped to breath fire as a dragon or spin webs as a spider. Any staffs or wands you carry do not use a charge if the monster you become can cast the same spell."},
	{"alchemy",				itemMaterials[12], "",	1,	1200,	0,{0,0,0}, false, false, "This stained catalyst lets you decant potions and organize scrolls into folios so that you can carry as many doses of the same potion or pages of the same scroll in your inventory as you need without requiring additional space. You may also apply this talisman to combine potions into elixirs and scrolls into tomes to boost their effectiveness - or use a scroll of enchanting for the same effect (although enchanting potions of life requires you have a lumenstone)."},
	{"witchcraft",			itemMaterials[13],"",	1,	1000,	0,{0,0,0}, false, false, "This improvised ward changes some of the potions and scrolls you discover into elixirs and tomes; and lets you change elixirs and tomes into charms by enchanting them."},
	{"the sinister hand",	itemMaterials[14], "",	1,	1000,	0,{0,0,0}, false, false, "This sinister bauble allows you to wield two weapons at once instead of a weapon and shield; at a cost of adding one third the strength requirement of your second weapon to all your equipment and doubling the time required to use a staff or wand."},
	{"the third eye",		itemMaterials[15], "",	1,	1000,	0,{0,0,0}, false, false, "This ancient symbol allows you to wear a third ring; at a cost of not being able to wear any armour."},
	{"assassins",			itemMaterials[16], "",	1,	900,	0,{0,0,0}, false, false, "This obscure mark allows you to apply your darts to potions to coat them for various effects and gives you an assassination target on each new level of the dungeon to kill; completing assassinations rewards you with gold or darts next to the dungeon stairs the next time you change level."},
};


#pragma mark Miscellaneous definitions

const color *boltColors[NUMBER_BOLT_KINDS] = {
	&darkGreen,			// force
	&lightningColor,	// lightning
	&fireBoltColor,		// fire
	&poisonColor,		// poison
	&brown,				// tunneling
	&darkOrange,		// detonation
	&white,				// blinking
	&spectralBladeColor,// conjuration
	&blue,				// teleport other
	&green,				// slow
	&pink,				// negation
	&dominationColor,	// domination
	&yellow,			// entrancement
	&forceFieldColor,	// obstruction
	&discordColor,		// discord
	&algaeGreenLightColor,// nature
	&rainbow,			// plenty
	&purple,			// polymorph
	&beckonColor,		// beckoning
	&darkBlue,			// invisibility
	&darkRed,			// healing
	&orange,			// haste
	&shieldingColor,	// shielding
	&lightBlue,			// reflective
	&darkGray,			// sentry
	&vomitColor,		// zombie
//	&veryDarkGray,		// phantom
	&poisonGasColor,	// bloat
	&gray,				// spider
	&trollColor,		// naga
	&pixieColor,		// pixie
	&toadColor,			// toad
	&wormColor,			// underworm
	&mandrakeRootColor,	// mandrake
	&methaneColor,		// dead man's ear
	&markedColor,		// crimson cap
	&glyphColor,		// mirrored totem
};

const char monsterBehaviorFlagDescriptions[32][COLS] = {
	"is invisible",								// MONST_INVISIBLE
	"is an inanimate object",					// MONST_INANIMATE
	"cannot move",								// MONST_IMMOBILE
	"carries an item",							// MONST_CARRY_ITEM_100
	"sometimes carries an item",				// MONST_CARRY_ITEM_25
	"never wanders",							// MONST_ALWAYS_HUNTING
	"flees at low health",						// MONST_FLEES_NEAR_DEATH
	"",											// MONST_ATTACKABLE_THRU_WALLS
	"corrodes weapons when hit",				// MONST_DEFEND_DEGRADE_WEAPON
	"is immune to physical damage",				// MONST_IMMUNE_TO_WEAPONS
	"flies",									// MONST_FLIES
	"moves erratically",						// MONST_FLITS
	"is immune to fire",						// MONST_IMMUNE_TO_FIRE
	"",											// MONST_CAST_SPELLS_SLOWLY
	"cannot be entangled",						// MONST_IMMUNE_TO_WEBS
	"can reflect magic spells",                 // MONST_REFLECT_4
	"never sleeps",								// MONST_NEVER_SLEEPS
	"burns unceasingly",						// MONST_FIERY
	"",											// MONST_INTRINSIC_LIGHT
	"is at home in water",						// MONST_IMMUNE_TO_WATER
	"cannot venture onto dry land",				// MONST_RESTRICTED_TO_LIQUID
	"submerges",								// MONST_SUBMERGES
	"keeps $HISHER distance",					// MONST_MAINTAINS_DISTANCE
	"",											// MONST_WILL_NOT_USE_STAIRS
	"is animated purely by magic",				// MONST_DIES_IF_NEGATED
	"",                                         // MONST_MALE
	"",                                         // MONST_FEMALE
    "",                                         // MONST_NOT_LISTED_IN_SIDEBAR
    "moves only when activated",                // MONST_GETS_TURN_ON_ACTIVATION
	"",											// MONST_ALWAYS_USE_ABILITY
	"carries a shield to protect $HIMSELFHERSELF",	// MONST_SHIELD_BLOCKS
};

const char monsterAbilityFlagDescriptions[33][COLS] = {
	"can induce hallucinations",				// MA_HIT_HALLUCINATE
	"can steal items",							// MA_HIT_STEAL_FLEE
	"can possess $HISHER summoned allies",		// MA_ENTER_SUMMONS
	"corrodes armor when $HESHE hits",			// MA_HIT_DEGRADE_ARMOR
	"can heal $HISHER allies",					// MA_CAST_HEAL
	"can haste $HISHER allies",					// MA_CAST_HASTE
	"can cast protection",						// MA_CAST_PROTECTION
	"can summon allies",						// MA_CAST_SUMMON
	"can blink towards enemies",				// MA_CAST_BLINK
	"can cast negation",						// MA_CAST_NEGATION
	"can throw sparks of lightning",			// MA_CAST_SPARK
	"can throw bolts of fire",					// MA_CAST_FIRE
	"can slow $HISHER enemies",					// MA_CAST_SLOW
	"can cast discord",							// MA_CAST_DISCORD
    "can cast beckoning",                       // MA_CAST_BECKONING
	"can breathe gouts of white-hot flame",		// MA_BREATHES_FIRE
	"can launch sticky webs",					// MA_SHOOTS_WEBS
	"attacks from a distance",					// MA_ATTACKS_FROM_DISTANCE
	"immobilizes $HISHER prey",					// MA_SEIZES
	"injects poison when $HESHE hits",			// MA_POISONS
	"",											// MA_DF_ON_DEATH
	"divides in two when struck",				// MA_CLONE_SELF_ON_DEFEND
	"dies when $HESHE attacks",					// MA_KAMIKAZE
	"recovers health when $HESHE inflicts damage",// MA_TRANSFERENCE
    "saps strength when $HESHE inflicts damage",// MA_CAUSE_WEAKNESS
	"can create turrets in walls nearby",		// MA_CAST_SENTRY
	"can activate nearby guardians",			// MA_CALL_GUARDIAN
	"can turn invisible if frightened",			// MA_INVISIBLE_IF_FLEES
	"can knock enemies backwards, for triple damage into walls", // MA_HIT_KNOCKS_BACK
	"can lunge for triple damage from 2 metres away",// MA_LUNGE_ATTACKS
	"can attack through one monster to a second",	// MA_ATTACKS_PENETRATE
	"can attack all adjacent to it with one blow",	// MA_ATTACKS_ALL_ADJACENT
};

const char monsterBookkeepingFlagDescriptions[32][COLS] = {
	"",											// MONST_WAS_VISIBLE
	"",											// unused
	"",											// MONST_PREPLACED
	"",											// MONST_APPROACHING_UPSTAIRS
	"",											// MONST_APPROACHING_DOWNSTAIRS
	"",											// MONST_APPROACHING_PIT
	"",											// MONST_LEADER
	"",											// MONST_FOLLOWER
	"",											// MONST_CAPTIVE
	"has been immobilized",						// MONST_SEIZED
	"is currently holding $HISHER prey immobile",// MONST_SEIZING
	"is submerged",								// MONST_SUBMERGED
	"",											// MONST_JUST_SUMMONED
	"",											// MONST_WILL_FLASH
	"is anchored to reality by $HISHER summoner",// MONST_BOUND_TO_LEADER
	"is a target you have a reward to kill",	// MONST_ASSASSINATION_TARGET
	"",											// MONST_ASSASSINATION_ALERT
	"",											// MONST_ASSASSINATION_ALARM
	"",											// MONST_ATTACK_SHALLOWS
	"",											// MONST_NO_EXPERIENCE
};
