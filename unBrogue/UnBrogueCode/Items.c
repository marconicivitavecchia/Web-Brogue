/*
 *  Items.c
 *  UnBrogue
 *
 *  Brogue created by Brian Walker on 1/10/09.
 *  Copyright 2013. All rights reserved.
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
#include "IncludeGlobals.h"
#include <math.h>

// Allocates space, generates a specified item (or random category/kind if -1)
// and returns a pointer to that item. The item is not given a location here
// and is not inserted into the item chain!
item *generateItem(unsigned long theCategory, short theKind) {
	short i;
	item *theItem;
	
	theItem = (item *) malloc(sizeof(item));
	memset(theItem, '\0', sizeof(item) );
	
	theItem->category = 0;
	theItem->kind = 0;
	theItem->flags = 0;
	theItem->displayChar = '&';
	theItem->foreColor = &itemColor;
	theItem->inventoryColor = &white;
	theItem->armor = 0;
	theItem->strengthRequired = 0;
	theItem->hiddenRunic = 0;
	theItem->hiddenRunicEnchantsRequired = 0;
	theItem->vorpalEnemy = 0;
	theItem->numberOfBoltEnchants = 0;
	theItem->charges = 0;
	theItem->quantity = 1;
	theItem->quiverNumber = 0;
	theItem->keyZ = 0;
	theItem->inscription[0] = '\0';
	theItem->nextItem = NULL;
	
	for (i=0; i < KEY_ID_MAXIMUM; i++) {
		theItem->keyLoc[i].x = 0;
		theItem->keyLoc[i].y = 0;
		theItem->keyLoc[i].machine = 0;
		theItem->keyLoc[i].disposableHere = false;
	}
	
	makeItemInto(theItem, theCategory, theKind);
	
	return theItem;
}

unsigned long pickItemCategory(unsigned long theCategory) {
	short i, sum, randIndex;
	short probabilities[NUMBER_ITEM_CATEGORIES] =					{46,	42,		0,		52,		0,		3,		3,		10,		8,		3,		5,		3,		2,		1,		0,		0,		0};
	unsigned long correspondingCategories[NUMBER_ITEM_CATEGORIES] =	{GOLD,	SCROLL,	TOME,	POTION,	ELIXIR,	STAFF,	WAND,	WEAPON,	ARMOR,	SHIELD,	FOOD,	RING,	CHARM,	TALISMAN,	AMULET,	GEM,	KEY};

	sum = 0;

	// adjust wand/staff probabilities based on number of wands/staffs -- ignore summoning items
	for (i = 0; i < BOLT_SENTRY; i++)
	{
		if (staffTable[i].frequency) sum++;
	}
	
	// round wand probabilities up, staffs down
	probabilities[5] = clamp(6 * sum / BOLT_SENTRY, 1, 5);
	probabilities[6] = 6 - probabilities[5];
	
	sum = 0;
	
	for (i=0; i<NUMBER_ITEM_CATEGORIES; i++) {
		if (theCategory & correspondingCategories[i] || theCategory <= 0) {
			sum += probabilities[i];
		}
	}
	
	if (sum == 0) {
		return theCategory; // e.g. when you pass in AMULET or GEM, since they have no frequency
	}
	
	randIndex = rand_range(1, sum);
	
	for (i=0; ; i++) {
		if (theCategory <= 0 || theCategory & correspondingCategories[i]) {
			if (randIndex <= probabilities[i]) {
				return correspondingCategories[i];
			}
			randIndex -= probabilities[i];
		}
	}
}

// true iff the combination of bolt runic and item is permitted
boolean boltRunicAllowedOnItem(short boltRune, short itemKind, item *theItem, boolean wizard) {
	short i;
	unsigned long boltHasFlags;
	
	if (theItem && (theItem->category & KEY) && (theItem->kind == KEY_RUNE_WAND || theItem->kind == KEY_RUNE_STAFF)) {
		return true;
	}	
	
	if (theItem && !(theItem->category & (STAFF | WAND))) {
		return false;
	}
	
	if (theItem && theItem->numberOfBoltEnchants > 2) {
		return false;
	}
	
	boltHasFlags = 0;
	if (theItem) {
		for (i = (wizard ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
			boltHasFlags |= boltRunicCatalog[theItem->boltEnchants[i]].flags;
		}
	}
	
	if (boltHasFlags & boltRunicCatalog[boltRune].forbiddenFlags) {
		return false;
	}
	
	// check the prevent all flags first. blinking also checked here.
	switch(itemKind) {
		case BOLT_FIRE:
		case BOLT_LIGHTNING:
		case BOLT_FORCE:
		case BOLT_POISON:
			return true;
		case BOLT_BLINKING:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_BLINKING) {
				return false;
			}
			return true;
		case BOLT_TUNNELING:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_TUNNELING) {
				return false;
			}
			return true;
		case BOLT_CONJURATION:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_CONJURING) {
				return false;
			}
			break;
		case BOLT_OBSTRUCTION:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_OBSTRUCTION) {
				return false;
			}
			break;
		case BOLT_NATURE:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_NATURE) {
				return false;
			}
			break;
		case BOLT_BECKONING:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_BECKONING) {
				return false;
			}
			break;
		case BOLT_POLYMORPH:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_POLYMORPH) {
				return false;
			}
			break;
		case BOLT_PLENTY:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_PLENTY) {
				return false;
			}
			break;
		case BOLT_SENTRY:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_SENTRY) {
				return false;
			}
			break;
		case BOLT_REFLECTION:
		case BOLT_HEALING:
		case BOLT_SHIELDING:
		case BOLT_HASTE:
		case BOLT_INVISIBILITY:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_ASSISTING) {
				return false;
			}
			break;
		case BOLT_UNDERWORM:
			if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_TUNNELING) {
				return false;
			}
			break;
		default:
			break;
	}
	if (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_ALL_BUT_ELEMENTAL) {
		return false;
	}
	
	if (itemKind >= BOLT_SENTRY && (boltRunicCatalog[boltRune].flags & BOLT_PREVENT_SUMMONING)) {
		return false;
	}
	
	return true;
}


// Sets an item to the given type and category (or chooses randomly if -1) with all other stats
item *makeItemInto(item *theItem, unsigned long itemCategory, short itemKind) {
	itemTable *theEntry;

	if (itemCategory <= 0) {
		itemCategory = ALL_ITEMS;
	}
	
	itemCategory = pickItemCategory(itemCategory);
	
	theItem->category = itemCategory;
	
	switch (itemCategory) {
			
		case FOOD:
			if (itemKind < 0) {
				itemKind = chooseKind(foodTable, NUMBER_FOOD_KINDS);
			}
			theEntry = &foodTable[itemKind];
			theItem->displayChar = FOOD_CHAR;
			theItem->flags |= ITEM_IDENTIFIED;
			break;
			
		case WEAPON:
			if (itemKind < 0) {
				itemKind = chooseKind(weaponTable, NUMBER_WEAPON_KINDS);
			}
			theEntry = &weaponTable[itemKind];
			theItem->damage = weaponTable[itemKind].range;
			theItem->strengthRequired = weaponTable[itemKind].strengthRequired;
			theItem->displayChar = WEAPON_CHAR;
			
			switch (itemKind) {
				case MACE:
					theItem->flags |= ITEM_BACKSTAB_XQUARTER;
				case HAMMER:
					theItem->flags |= (ITEM_ATTACKS_SLOWLY | ITEM_HIT_KNOCKBACK);
					break;
				case SABRE:
					theItem->flags |= ITEM_ATTACKS_ALL_ADJACENT;
				case RAPIER:
					theItem->flags |= (ITEM_ATTACKS_QUICKLY | ITEM_LUNGE_ATTACKS);
					// could give rapier backstab bonus to make it better than sabre against single targets
					break;
				case SPEAR:
					theItem->flags |= ITEM_BACKSTAB_XHALF;
				case PIKE:
					theItem->flags |= (ITEM_ATTACKS_PENETRATE | ITEM_IMPALES);
					break;
				case AXE:
					theItem->flags |= ITEM_BACKSTAB_XHALF;
				case WAR_AXE:
					theItem->flags |= ITEM_ATTACKS_ALL_ADJACENT;
					break;
				case DAGGER:
					theItem->flags |= ITEM_BACKSTAB_X2;
					break;
				case SWORD:
					theItem->flags |= ITEM_BACKSTAB_X1;
					break;
				case BROADSWORD:
					theItem->flags |= ITEM_BACKSTAB_XQUARTER;
					break;
				default:
					break;
			}
			
			if (rand_percent(40)) {
				theItem->enchant1 += rand_range(1, 3);
				if (rand_percent(50)) {
					// cursed
					theItem->enchant1 *= -1;
					theItem->flags |= ITEM_CURSED;
					if (rand_percent(33)) { // give it a bad runic
						theItem->enchant2 = rand_range(NUMBER_GOOD_WEAPON_ENCHANT_KINDS, NUMBER_WEAPON_RUNIC_KINDS - 1);
						theItem->flags |= ITEM_RUNIC;
					}
				} else if (rand_range(3, 10) * ((theItem->flags & ITEM_ATTACKS_SLOWLY) ? 2 : 1)
						   / ((theItem->flags & ITEM_ATTACKS_QUICKLY) ? 2 : 1) > theItem->damage.lowerBound) {
					// give it a good runic; lower damage items are more likely to be runic
					theItem->enchant2 = rand_range(0, NUMBER_GOOD_WEAPON_ENCHANT_KINDS - 1);
					theItem->flags |= ITEM_RUNIC;
					//if (theItem->enchant2 == W_SLAYING) {
						theItem->vorpalEnemy = chooseVorpalEnemy();
					//}
				}
			}
			if (itemKind >= DART) {
				theItem->quantity = rand_range(3, 6); // we start with 3 darts
				theItem->quiverNumber = rand_range(1, 60000);
				theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC); // throwing weapons can't be cursed or runic
				theItem->enchant1 = 0; // throwing weapons can't be magical
			}
			theItem->charges = WEAPON_KILLS_TO_AUTO_ID; // kill 20 enemies to auto-identify
			if (!(theItem->flags & ITEM_RUNIC)) {
				if ((rand_range(3, 10) * ((theItem->flags & ITEM_ATTACKS_SLOWLY) ? 2 : 1)
					   / ((theItem->flags & ITEM_ATTACKS_QUICKLY) ? 2 : 1) <= theItem->damage.lowerBound)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				if ((rand_range(3, 10) * ((theItem->flags & ITEM_ATTACKS_SLOWLY) ? 2 : 1)
																  / ((theItem->flags & ITEM_ATTACKS_QUICKLY) ? 2 : 1) <= theItem->damage.lowerBound)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				theItem->hiddenRunicEnchantsRequired += rand_range(1, 4);
				theItem->hiddenRunic = rand_range(0, NUMBER_GOOD_WEAPON_ENCHANT_KINDS - 1);
				//if (theItem->hiddenRunic == W_SLAYING) {
					theItem->vorpalEnemy = chooseVorpalEnemy();
				//}
			}
			break;
			
		case ARMOR:
			if (itemKind < 0) {
				itemKind = chooseKind(armorTable, NUMBER_ARMOR_KINDS);
			}
			theEntry = &armorTable[itemKind];
			theItem->armor = randClump(armorTable[itemKind].range);
			theItem->strengthRequired = armorTable[itemKind].strengthRequired;
			theItem->displayChar = ARMOR_CHAR;
			theItem->charges = ARMOR_DELAY_TO_AUTO_ID; // this many turns until it reveals its enchants and whether runic
			if (rand_percent(40)) {
				theItem->enchant1 += rand_range(1, 3);
				if (rand_percent(50)) {
					// cursed
					theItem->enchant1 *= -1;
					theItem->flags |= ITEM_CURSED;
					if (rand_percent(33)) { // give it a bad runic
						theItem->enchant2 = rand_range(NUMBER_GOOD_ARMOR_ENCHANT_KINDS, NUMBER_ARMOR_ENCHANT_KINDS - 1);
						theItem->flags |= ITEM_RUNIC;
					}
				} else if (rand_range(0, 95) + (theItem->armor > 30 ? 10 : 0) > theItem->armor) { // give it a good runic
					theItem->enchant2 = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
					theItem->flags |= ITEM_RUNIC;
					if (theItem->enchant2 == A_IMMUNITY) {
						theItem->vorpalEnemy = chooseVorpalEnemy();
					}
				}
			}
			if (!(theItem->flags & ITEM_RUNIC)) {
				if ((rand_range(0, 95) + (theItem->armor > 30 ? 10 : 0) <= theItem->armor)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				if ((rand_range(0, 95) + (theItem->armor > 30 ? 10 : 0) <= theItem->armor)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				theItem->hiddenRunicEnchantsRequired += rand_range(1, 4);
				theItem->hiddenRunic = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
				//if (theItem->hiddenRunic == A_IMMUNITY) {
					theItem->vorpalEnemy = chooseVorpalEnemy();
				//}
			}
			break;
		case SHIELD: // uses armor runics
			if (itemKind < 0) {
				itemKind = chooseKind(shieldTable, NUMBER_SHIELD_KINDS);
			}
			theEntry = &shieldTable[itemKind];
			theItem->shieldBlows = shieldTable[itemKind].range.lowerBound;
			theItem->shieldChance = shieldTable[itemKind].range.upperBound;
			theItem->shieldMinBlow = shieldTable[itemKind].range.clumpFactor;
			theItem->strengthRequired = shieldTable[itemKind].strengthRequired;
			theItem->displayChar = SHIELD_CHAR;
			theItem->charges = SHIELD_DELAY_TO_AUTO_ID; // this many turns until it reveals its enchants and whether runic
			if (rand_percent(40)) {
				theItem->enchant1 += rand_range(1, 3);
				if (rand_percent(50)) {
					// cursed
					theItem->enchant1 *= -1;
					theItem->flags |= ITEM_CURSED;
					if (rand_percent(33)) { // give it a bad runic
						theItem->enchant2 = rand_range(NUMBER_GOOD_ARMOR_ENCHANT_KINDS, NUMBER_ARMOR_ENCHANT_KINDS - 1);
						theItem->flags |= ITEM_RUNIC;
					}
				} else if (rand_range(0, 50) > theItem->shieldChance) { // give it a good runic
					theItem->enchant2 = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
					theItem->flags |= ITEM_RUNIC;
					if (theItem->enchant2 == A_IMMUNITY) {
						theItem->vorpalEnemy = chooseVorpalEnemy();
					}
				}
			}
			if (!(theItem->flags & ITEM_RUNIC)) {
				if ((rand_range(0, 50) < theItem->shieldChance)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				if ((rand_range(0, 50) < theItem->shieldChance)) {
					theItem->hiddenRunicEnchantsRequired++;
				}
				theItem->hiddenRunicEnchantsRequired += rand_range(1, 4);
				theItem->hiddenRunic = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
				//if (theItem->hiddenRunic == A_IMMUNITY) {
					theItem->vorpalEnemy = chooseVorpalEnemy();
				//}
			}
			break;
		case SCROLL:
			if (itemKind < 0) {
				itemKind = chooseKind(scrollTable, NUMBER_SCROLL_KINDS);
			}
			if (rogue.witchcraftChance) {  // for promotion with talisman of witchcraft
				if (rand_percent(rogue.witchcraftChance)) {
					theItem->flags |= ITEM_OVERCHARGED;
					rogue.witchcraftChance = 10;
				} else {
					rogue.witchcraftChance += 10;
				}
			}
			theEntry = &scrollTable[itemKind];
			theItem->displayChar = SCROLL_CHAR;
			theItem->flags |= ITEM_FLAMMABLE;
			break;
		case TOME:
			if (itemKind < 0) {
				itemKind = chooseKind(tomeTable, NUMBER_TOME_KINDS);
			}
			theEntry = &tomeTable[itemKind];
			theItem->displayChar = TOME_CHAR;
			break;
		case POTION:
			if (itemKind < 0) {
				itemKind = chooseKind(potionTable, NUMBER_POTION_KINDS);
			}
			if (rogue.witchcraftChance) {  // for promotion with talisman of witchcraft
				if (rand_percent(rogue.witchcraftChance)) {
					theItem->flags |= ITEM_OVERCHARGED;
					rogue.witchcraftChance = 10;
				} else {
					rogue.witchcraftChance += 10;
				}
			}
			theEntry = &potionTable[itemKind];
			theItem->displayChar = POTION_CHAR;
			break;
		case ELIXIR:
			if (itemKind < 0) {
				itemKind = chooseKind(elixirTable, NUMBER_ELIXIR_KINDS);
			}
			theEntry = &elixirTable[itemKind];
			theItem->displayChar = ELIXIR_CHAR;
			break;
		case STAFF:
			if (itemKind < 0) {
				itemKind = chooseKind(staffTable, NUMBER_STAFF_KINDS);
			}
			theEntry = &staffTable[itemKind];
			theItem->displayChar = STAFF_CHAR;
			theItem->charges = 2;
			if (rand_percent(50)) {
				theItem->charges++;
				if (rand_percent(15)) {
					theItem->charges++;
				}
			}
			do {
				theItem->boltEnchants[0] = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS-1);
			} while (!boltRunicAllowedOnItem(theItem->boltEnchants[0], itemKind, theItem, false));
			if (rand_percent(theItem->kind < STAFF_CONJURATION ? 20 : 10)) {
				do {
					theItem->boltEnchants[1] = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS-1);
				} while (!boltRunicAllowedOnItem(theItem->boltEnchants[1], itemKind, theItem, false));
				theItem->numberOfBoltEnchants++;
			}
			theItem->enchant1 = theItem->charges;
			theItem->enchant2 = (itemKind == STAFF_BLINKING || itemKind == STAFF_OBSTRUCTION ? 1000 : 500); // start with no recharging mojo
			break;
		case WAND:
			if (itemKind < 0) {
				itemKind = chooseKind(wandTable, NUMBER_WAND_KINDS);
			}
			theEntry = &wandTable[itemKind];
			theItem->displayChar = WAND_CHAR;
			theItem->charges = randClump(wandTable[itemKind].range);
			do {
				theItem->boltEnchants[0] = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS-1);
			} while (!boltRunicAllowedOnItem(theItem->boltEnchants[0], itemKind, theItem, false));
			if (rand_percent(theItem->kind < WAND_CONJURATION ? 20 : 10)) {
				do {
					theItem->boltEnchants[1] = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS-1);
				} while (!boltRunicAllowedOnItem(theItem->boltEnchants[1], itemKind, theItem, false));
				theItem->numberOfBoltEnchants++;
			}
			break;
		case RING:
			if (itemKind < 0) {
				itemKind = chooseKind(ringTable, NUMBER_RING_KINDS);
			}
			theEntry = &ringTable[itemKind];
			theItem->displayChar = RING_CHAR;
			theItem->enchant1 = randClump(ringTable[itemKind].range);
			theItem->charges = RING_DELAY_TO_AUTO_ID; // how many turns of being worn until it auto-identifies
			if (rand_percent(16)) {
				// cursed
				theItem->enchant1 *= -1;
				theItem->flags |= ITEM_CURSED;
			}
			break;
        case CHARM:
			if (itemKind < 0) {
				itemKind = chooseKind(charmTable, NUMBER_CHARM_KINDS);
			}
			theEntry = &charmTable[itemKind];
            theItem->displayChar = CHARM_CHAR;
            theItem->charges = 0; // Charms are initially ready for use.
            theItem->enchant1 = randClump(charmTable[itemKind].range);
            break;
		case TALISMAN:
			if (itemKind < 0) {
				itemKind = chooseKind(talismanTable, NUMBER_TALISMAN_KINDS);
			}
			theEntry = &talismanTable[itemKind];
			theItem->displayChar = TALISMAN_CHAR;
			if (itemKind <= TALISMAN_MAX_ENCHANT) {
				theItem->enchant1 = randClump(talismanTable[itemKind].range);
				if (/*itemKind != TALISMAN_TRAP_MASTERY &&*/ rand_percent(16)) { // trap mastery cannot be cursed
					// cursed
					theItem->enchant1 *= -1;
					theItem->flags |= ITEM_CURSED;
				}
			}
			if (itemKind >= TALISMAN_MIN_AUTOIDENTIFY) {
				theItem->flags |= (ITEM_KIND_AUTO_ID);
			}
			break;
		case GOLD:
			theEntry = NULL;
			theItem->displayChar = GOLD_CHAR;
			theItem->quantity = rand_range(50 + rogue.depthLevel * 10, 100 + rogue.depthLevel * 15);
			break;
		case AMULET:
			theEntry = NULL;
			theItem->displayChar = AMULET_CHAR;
			itemKind = 0;
			theItem->flags |= ITEM_IDENTIFIED;
			break;
		case GEM:
			theEntry = NULL;
			theItem->displayChar = GEM_CHAR;
			itemKind = 0;
			theItem->flags |= ITEM_IDENTIFIED;
			break;
		case KEY:
			theEntry = NULL;
			theItem->displayChar = KEY_CHAR;
			theItem->flags |= (ITEM_IDENTIFIED | ITEM_NAMED | ITEM_PLAYER_AVOIDS);
			if (itemKind == KEY_RUNE_ARMOR) {
				theItem->enchant2 = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
				theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
				if (theItem->enchant2 == A_IMMUNITY) {
					theItem->vorpalEnemy = chooseVorpalEnemy();
				}
			}
			if (itemKind == KEY_RUNE_WEAPON) {
				theItem->enchant2 = rand_range(0, NUMBER_GOOD_WEAPON_ENCHANT_KINDS - 1);
				theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
				if (theItem->enchant2 == W_SLAYING) {
					theItem->vorpalEnemy = chooseVorpalEnemy();
				}
			}
			if (itemKind == KEY_RUNE_STAFF) {
				theItem->enchant2 = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS - 1);
				theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
			}
			if (itemKind == KEY_RUNE_WAND) {
				theItem->enchant2 = rand_range(0, NUMBER_BOLT_ENCHANT_KINDS - 1);
				theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
			}
			break;
		default:
			theEntry = NULL;
			message("something has gone terribly wrong!", true);
			break;
	}
	if (theItem
		&& !(theItem->flags & ITEM_IDENTIFIED)
		&& (!(theItem->category & (POTION | SCROLL | ELIXIR | TOME ) ) || (theEntry && !theEntry->identified))) {
		theItem->flags |= ITEM_CAN_BE_IDENTIFIED;
	}
	theItem->kind = itemKind;
	
	return theItem;
}

short chooseKind(itemTable *theTable, short numKinds) {
	short i, totalFrequencies = 0, randomFrequency;
	for (i=0; i<numKinds; i++) {
		totalFrequencies += max(0, theTable[i].frequency);
	}
	randomFrequency = rand_range(1, totalFrequencies);
	for (i=0; randomFrequency > theTable[i].frequency; i++) {
		randomFrequency -= max(0, theTable[i].frequency);
	}
	return i;
}

// Places an item at (x,y) if provided or else a random location if they're 0. Inserts item into the floor list.
item *placeItem(item *theItem, short x, short y) {
	short loc[2];
	enum dungeonLayers layer;
	char theItemName[DCOLS], buf[DCOLS];
	if (x <= 0 || y <= 0) {
		randomMatchingLocation(&(loc[0]), &(loc[1]), FLOOR, NOTHING, -1);
		theItem->xLoc = loc[0];
		theItem->yLoc = loc[1];
	} else {
		theItem->xLoc = x;
		theItem->yLoc = y;
	}
	
	removeItemFromChain(theItem, floorItems); // just in case; double-placing an item will result in game-crashing loops in the item list
	
	theItem->nextItem = floorItems->nextItem;
	floorItems->nextItem = theItem;
	pmap[theItem->xLoc][theItem->yLoc].flags |= HAS_ITEM;
	if ((theItem->flags & ITEM_MAGIC_DETECTED) && itemMagicChar(theItem)) {
		pmap[theItem->xLoc][theItem->yLoc].flags |= ITEM_DETECTED;
	}
	if (cellHasTerrainFlag(x, y, T_IS_DF_TRAP)
		&& !(pmap[x][y].flags & PRESSURE_PLATE_DEPRESSED)) {
		
		pmap[x][y].flags |= PRESSURE_PLATE_DEPRESSED;
		if (playerCanSee(x, y)) {
			if (cellHasTerrainFlag(x, y, T_IS_SECRET)) {
				discover(x, y);
				refreshDungeonCell(x, y);
			}
			itemName(theItem, theItemName, false, false, NULL);
			sprintf(buf, "a pressure plate clicks underneath the %s!", theItemName);
			message(buf, true);
		}
		for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
			if (tileCatalog[pmap[x][y].layers[layer]].flags & T_IS_DF_TRAP) {
				spawnDungeonFeature(x, y, &(dungeonFeatureCatalog[tileCatalog[pmap[x][y].layers[layer]].fireType]), true, false);
				promoteTile(x, y, layer, false);
			}
		}
	}
	// Tile is promoted by a matching item kind
	if (cellHasTerrainFlag2(x, y, T2_PROMOTES_ON_DROP)
		// Restrict keys to tiles that they match
		&& (!(theItem->category & KEY) || keyOnTileAt(x, y))) {
		for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
			if (tileCatalog[pmap[x][y].layers[layer]].flags2 & theItem->category) {
				
				if (playerCanSee(x, y)) {
					if (cellHasTerrainFlag(x, y, T_IS_SECRET)) {
						discover(x, y);
						refreshDungeonCell(x, y);
					}
					itemName(theItem, theItemName, false, false, NULL);
					sprintf(buf, "the %s fits perfectly!", theItemName);
					message(buf, true);
				}
				promoteTile(x, y, layer, false);
				return NULL; // theItem might be lost through promotion, so we can't return it.
			}
		}		
	}
	return theItem;
}

void fillItemSpawnHeatMap(unsigned short heatMap[DCOLS][DROWS], unsigned short heatLevel, short x, short y) {
	enum directions dir;
	short newX, newY;
	
	if (pmap[x][y].layers[DUNGEON] == DOOR) {
		heatLevel += 10;
	} else if (pmap[x][y].layers[DUNGEON] == SECRET_DOOR) {
		heatLevel += 3000;
	}
	if (heatMap[x][y] > heatLevel) {
		heatMap[x][y] = heatLevel;
	}
	for (dir = 0; dir < 4; dir++) {
		newX = x + nbDirs[dir][0];
		newY = y + nbDirs[dir][1];
		if (coordinatesAreInMap(newX, newY)
			&& (!cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY | T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_AUTO_DESCENT)
				|| cellHasTerrainFlag(newX, newY, T_IS_SECRET))
			&& heatLevel < heatMap[newX][newY]) {
			fillItemSpawnHeatMap(heatMap, heatLevel, newX, newY);
		}
	}
}

void coolHeatMapAt(unsigned short heatMap[DCOLS][DROWS], short x, short y, unsigned long *totalHeat) {
	short k, l;
	unsigned short currentHeat;
	
	currentHeat = heatMap[x][y];
	*totalHeat -= heatMap[x][y];
	heatMap[x][y] = 0;
	
	// lower the heat near the chosen location
	for (k = -5; k <= 5; k++) {
		for (l = -5; l <= 5; l++) {
			if (coordinatesAreInMap(x+k, y+l) && heatMap[x+k][y+l] == currentHeat) {
				heatMap[x+k][y+l] = max(1, heatMap[x+k][y+l]/10);
				*totalHeat -= (currentHeat - heatMap[x+k][y+l]);
			}
		}
	}
}

// Returns false if no place could be found.
// That should happen only if the total heat is zero.
boolean getItemSpawnLoc(unsigned short heatMap[DCOLS][DROWS], short *x, short *y, unsigned long *totalHeat) {
	unsigned long randIndex;
	unsigned short currentHeat;
	short i, j;
	
	if (*totalHeat <= 0) {
		return false;
	}
	
	randIndex = rand_range(1, *totalHeat);
	
	//printf("\nrandIndex: %i", randIndex);
	
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			currentHeat = heatMap[i][j];
			if (randIndex <= currentHeat) { // this is the spot!
				*x = i;
				*y = j;
				return true;
			}
			randIndex -= currentHeat;
		}
	}
#ifdef BROGUE_ASSERTS
	assert(0); // should never get here!
#endif
	return false;
}

#define aggregateGoldLowerBound(d)	(pow((double) (d), 3.05) + 320 * (d))
#define aggregateGoldUpperBound(d)	(pow((double) (d), 3.05) + 420 * (d))

// Generates and places items for the level. Must pass the location of the up-stairway on the level.
void populateItems(short upstairsX, short upstairsY) {
	if (!ITEMS_ENABLED) {
		return;
	}
	item *theItem;
	unsigned short itemSpawnHeatMap[DCOLS][DROWS];
	short i, j, numberOfItems, numberOfGoldPiles, goldBonusProbability, x = 0, y = 0;
	unsigned long totalHeat;
	unsigned long theCategory;
	short theKind;
	
#ifdef AUDIT_RNG
	char RNGmessage[100];
#endif
	
	if (rogue.depthLevel > AMULET_LEVEL) {
        if (rogue.depthLevel - AMULET_LEVEL - 1 > 5) {
            numberOfItems = 1;
        } else {
            const short lumenstoneDistribution[6] = {3, 3, 3, 2, 2, 2};
            numberOfItems = lumenstoneDistribution[rogue.depthLevel - AMULET_LEVEL - 1];
        }
		numberOfGoldPiles = 0;
	} else {
		rogue.lifePotionFrequency += 34;
		rogue.strengthPotionFrequency += 17;
		rogue.enchantScrollFrequency += 30;
		numberOfItems = 3;
		while (rand_percent(60)) {
			numberOfItems++;
		}
		if (rogue.depthLevel <= 3) {
			numberOfItems += 2; // 6 extra items to kickstart your career as a rogue
		} else if (rogue.depthLevel <= 5) {
			numberOfItems++; // and 2 more here
		}

		numberOfGoldPiles = min(5, (int) (rogue.depthLevel / 4));
		for (goldBonusProbability = 60;
			 rand_percent(goldBonusProbability) && numberOfGoldPiles <= 10;
			 goldBonusProbability -= 15) {
			
			numberOfGoldPiles++;
		}
		// Adjust the amount of gold if we're past depth 5 and we were below or above
		// the production schedule as of the previous depth.
		if (rogue.depthLevel > 5) {
			if (rogue.goldGenerated < aggregateGoldLowerBound(rogue.depthLevel - 1)) {
				numberOfGoldPiles += 2;
			} else if (rogue.goldGenerated > aggregateGoldUpperBound(rogue.depthLevel - 1)) {
				numberOfGoldPiles -= 2;
			}
		}
	}
	
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			itemSpawnHeatMap[i][j] = 50000;
		}
	}
	fillItemSpawnHeatMap(itemSpawnHeatMap, 5, upstairsX, upstairsY);
	totalHeat = 0;
	
#ifdef AUDIT_RNG
	sprintf(RNGmessage, "\n\nInitial heat map for level %i:\n", rogue.currentTurnNumber);
	RNGLog(RNGmessage);
#endif
	
	for (j=0; j<DROWS; j++) {
		for (i=0; i<DCOLS; i++) {
			
			if (cellHasTerrainFlag(i, j, T_OBSTRUCTS_ITEMS | T_PATHING_BLOCKER)
				|| (pmap[i][j].flags & (IS_CHOKEPOINT | IN_LOOP | IS_IN_MACHINE))
				|| passableArcCount(i, j) > 1) { // not in hallways or quest rooms, please
				itemSpawnHeatMap[i][j] = 0;
			} else if (itemSpawnHeatMap[i][j] == 50000) {
				itemSpawnHeatMap[i][j] = 0;
				pmap[i][j].layers[DUNGEON] = PERM_WALL; // due to a bug that created occasional isolated one-cell islands;
				// not sure if it's still around, but this is a good-enough failsafe
			}
#ifdef AUDIT_RNG
			sprintf(RNGmessage, "%u%s%s\t%s",
					itemSpawnHeatMap[i][j],
					((pmap[i][j].flags & IS_CHOKEPOINT) ? " (C)": ""), // chokepoint
					((pmap[i][j].flags & IN_LOOP) ? " (L)": ""), // loop
					(i == DCOLS-1 ? "\n" : ""));
			RNGLog(RNGmessage);
#endif
			totalHeat += itemSpawnHeatMap[i][j];
		}
	}

	if (D_INSPECT_LEVELGEN) {
		short **map = allocDynamicGrid();
		for (i=0; i<DCOLS; i++) {
			for (j=0; j<DROWS; j++) {
				map[i][j] = itemSpawnHeatMap[i][j] * -1;
			}
		}
		dumpLevelToScreen();
		displayMap(map);
		freeDynamicGrid(map);
		message("Item spawn heat map:", true);
	}
	
	for (i=0; i<numberOfItems; i++) {
		theCategory = ALL_ITEMS & ~GOLD; // gold is placed separately, below, so it's not a punishment
		theKind = -1;
		
		scrollTable[SCROLL_ENCHANTING].frequency = rogue.enchantScrollFrequency;
		potionTable[POTION_STRENGTH].frequency = rogue.strengthPotionFrequency;
        potionTable[POTION_LIFE].frequency = rogue.lifePotionFrequency;
		rogue.witchcraftChance = 15;
		
		// Adjust the desired item category if necessary.
		if ((rogue.foodSpawned + foodTable[RATION].strengthRequired / 2) * 3
			<= pow(rogue.depthLevel, 1.3) * foodTable[RATION].strengthRequired * 0.55) {
			// guarantee a certain nutrition minimum of the equivalent of one ration every three levels,
			// with more food on deeper levels since they generally take more turns to complete
			theCategory = FOOD;
			if (rogue.depthLevel > AMULET_LEVEL) {
				numberOfItems++;
			}
		} else if (rogue.depthLevel > AMULET_LEVEL) {
			theCategory = GEM;
		} else if (rogue.lifePotionsSpawned * 4 + 3 < rogue.depthLevel) {
            theCategory = POTION;
            theKind = POTION_LIFE;
        }
		
		// Generate the item.
		theItem = generateItem(theCategory, theKind);
		
		if (theItem->category & FOOD) {
			rogue.foodSpawned += foodTable[theItem->kind].strengthRequired;
		}
		
		// Choose a placement location not in a hallway.
		do {
			if ((theItem->category & FOOD) || ((theItem->category & POTION) && theItem->kind == POTION_STRENGTH)) {
				randomMatchingLocation(&x, &y, FLOOR, NOTHING, -1); // food and gain strength don't follow the heat map
			} else {
				getItemSpawnLoc(itemSpawnHeatMap, &x, &y, &totalHeat);
			}
		} while (passableArcCount(x, y) > 1);
#ifdef BROGUE_ASSERTS
		assert(coordinatesAreInMap(x, y));
#endif
		// Cool off the item spawning heat map at the chosen location:
		coolHeatMapAt(itemSpawnHeatMap, x, y, &totalHeat);
		
		// Regulate the frequency of enchantment scrolls and strength/life potions.
		if ((theItem->category & SCROLL) && theItem->kind == SCROLL_ENCHANTING) {
			rogue.enchantScrollFrequency -= 50;
			//DEBUG printf("\ndepth %i: %i enchant scrolls generated so far", rogue.depthLevel, ++enchantScrolls);
		} else if ((theItem->category & POTION) && theItem->kind == POTION_LIFE) {
			//DEBUG printf("\n*** Depth %i: generated a life potion at %i frequency!", rogue.depthLevel, rogue.lifePotionFrequency);
			rogue.lifePotionFrequency -= 150;
            rogue.lifePotionsSpawned++;
		} else if ((theItem->category & POTION) && theItem->kind == POTION_STRENGTH) {
			//DEBUG printf("\n*** Depth %i: generated a strength potion at %i frequency!", rogue.depthLevel, rogue.strengthPotionFrequency);
			rogue.strengthPotionFrequency -= 50;
		}
		
		// Place the item.
		placeItem(theItem, x, y); // Random valid location already obtained according to heat map.
		
		if (D_INSPECT_LEVELGEN) {
			short **map = allocDynamicGrid();
			short i2, j2;
			for (i2=0; i2<DCOLS; i2++) {
				for (j2=0; j2<DROWS; j2++) {
					map[i2][j2] = itemSpawnHeatMap[i2][j2] * -1;
				}
			}
			dumpLevelToScreen();
			displayMap(map);
			freeDynamicGrid(map);
			plotCharWithColor(theItem->displayChar, mapToWindowX(x), mapToWindowY(y), black, purple);
			message("Added an item.", true);
		}
	}
	
	// Now generate gold.
	for (i=0; i<numberOfGoldPiles; i++) {
		theItem = generateItem(GOLD, -1);
		getItemSpawnLoc(itemSpawnHeatMap, &x, &y, &totalHeat);
		coolHeatMapAt(itemSpawnHeatMap, x, y, &totalHeat);
		placeItem(theItem, x, y);
		rogue.goldGenerated += theItem->quantity;
	}
	
	if (D_INSPECT_LEVELGEN) {
		dumpLevelToScreen();
		message("Added gold.", true);
	}
	
	scrollTable[SCROLL_ENCHANTING].frequency	= 0;	// No enchant scrolls or strength/life potions can spawn except via initial
	potionTable[POTION_STRENGTH].frequency      = 0;	// item population or blueprints that create them specifically.
    potionTable[POTION_LIFE].frequency          = 0;
	rogue.witchcraftChance = 0;							// No boosting of items for talisman of witchcraft
	
	//DEBUG printf("\nD:%i: %lu gold generated so far.", rogue.depthLevel, rogue.goldGenerated);
}

// Name of this function is a bit misleading -- basically returns true iff the item will stack without consuming an extra slot
// i.e. if it's a throwing weapon with a sibling already in your pack. False for potions and scrolls unless you have the
// talisman of alchemy.
boolean itemWillStackWithPack(item *theItem) {
	item *tempItem;
	if (rogue.talisman && rogue.talisman->kind == TALISMAN_ALCHEMY
		&& (theItem->category & (POTION | SCROLL)) != 0) {
		for (tempItem = packItems->nextItem;
			 tempItem != NULL && (tempItem->category != theItem->category || tempItem->kind != theItem->kind);
			 tempItem = tempItem->nextItem);
		return (tempItem ? true : false);
	} else if (!(theItem->quiverNumber)) {
		return false;
	} else {
		for (tempItem = packItems->nextItem;
			 tempItem != NULL && tempItem->quiverNumber != theItem->quiverNumber;
			 tempItem = tempItem->nextItem);
		return (tempItem ? true : false);
	}
}

void removeItemFrom(short x, short y) {
	short layer;
	
	pmap[x][y].flags &= ~HAS_ITEM;
	
	if (cellHasTerrainFlag2(x, y, T2_PROMOTES_ON_ITEM_PICKUP)) {
		for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
			if (tileCatalog[pmap[x][y].layers[layer]].flags2 & T2_PROMOTES_ON_ITEM_PICKUP) {
				promoteTile(x, y, layer, false);
			}
		}
	}
}

// adds the item at (x,y) to the pack
void pickUpItemAt(short x, short y) {
	item *theItem;
	char buf[COLS], buf2[COLS];
	
	rogue.disturbed = true;
	
	// find the item
	theItem = itemAtLoc(x, y);
	
	if (!theItem) {
		message("Error: Expected item; item not found.", true);
		return;
	}
	
	theItem->flags |= ITEM_NAMED;
	
	if ((theItem->flags & ITEM_KIND_AUTO_ID)
        && tableForItemCategory(theItem->category)
		&& !(tableForItemCategory(theItem->category)[theItem->kind].identified)) {
        
        identifyItemKind(theItem);
	}
	
	if (numberOfItemsInPack() < MAX_PACK_ITEMS || (theItem->category & GOLD) || itemWillStackWithPack(theItem)) {
		
		if (theItem->flags & ITEM_NO_PICKUP) { // no longer used			
			if (!(theItem->flags & ITEM_DORMANT)) {
				itemName(theItem, buf2, true, false, NULL); // include suffix but not article
				sprintf(buf, "the %s is stuck to the ground.", buf2);
				messageWithColor(buf, &itemMessageColor, false);
			}
			return;
		}
		
		// remove from floor chain
		pmap[x][y].flags &= ~ITEM_DETECTED;
		
#ifdef BROGUE_ASSERTS
		assert(removeItemFromChain(theItem, floorItems));
#else
		removeItemFromChain(theItem, floorItems);
#endif
		
		if (theItem->category & GOLD) {
			rogue.gold += theItem->quantity; 
			sprintf(buf, "you found %i pieces of gold.", theItem->quantity);
			messageWithColor(buf, &itemMessageColor, false);
			deleteItem(theItem);
			removeItemFrom(x, y); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP
			return;
		}
		
		if ((theItem->category & AMULET) && numberOfMatchingPackItems(AMULET, 0, 0, false)) {
			message("you already have the Amulet of Yendor.", false); 
			deleteItem(theItem);
			return;
		}
		
		theItem = addItemToPack(theItem);
		
		itemName(theItem, buf2, true, true, NULL); // include suffix, article
		
		sprintf(buf, "you now have %s (%c).", buf2, theItem->inventoryLetter);
		messageWithColor(buf, &itemMessageColor, false);
		
		removeItemFrom(x, y); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP
	} else {
		theItem->flags |= ITEM_PLAYER_AVOIDS; // explore shouldn't try to pick it up more than once.
		itemName(theItem, buf2, false, true, NULL); // include article
		sprintf(buf, "Your pack is too full to pick up %s.", buf2);
		message(buf, false);
	}
}

void conflateItemCharacteristics(item *newItem, item *oldItem) {
    
    // let magic detection and other flags propagate to the new stack...
    newItem->flags |= (oldItem->flags & (ITEM_MAGIC_DETECTED | ITEM_IDENTIFIED | ITEM_PROTECTED | ITEM_NAMED | ITEM_RUNIC
										 | ITEM_RUNIC_HINTED | ITEM_CAN_BE_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN));
    
    // keep the higher enchantment and lower strength requirement...
    if (oldItem->enchant1 > newItem->enchant1) {
        newItem->enchant1 = oldItem->enchant1;
    }
    if (oldItem->strengthRequired < newItem->strengthRequired) {
        newItem->strengthRequired = oldItem->strengthRequired;
    }
    // Copy the inscription.
    if (oldItem->inscription && !newItem->inscription) {
        strcpy(newItem->inscription, oldItem->inscription);
    }
}

void stackItems(item *newItem, item *oldItem) {
    //Increment the quantity of the old item...
    newItem->quantity++;
    
    // ...conflate attributes...
    conflateItemCharacteristics(newItem, oldItem);
    
    // ...and delete the new item.
    deleteItem(oldItem);
}

item *addItemToPack(item *theItem) {
	item *previousItem, *tempItem;
	char itemLetter;
	
	theItem->flags |= ITEM_NAMED;
	
	// Can the item stack with another in the inventory?
	if (theItem->category & (FOOD|POTION|ELIXIR|SCROLL|TOME|GEM)) {
		for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
			if (theItem->category == tempItem->category && theItem->kind == tempItem->kind) {
				// We found a match!
                stackItems(tempItem, theItem);
				
				// Pass back the incremented (old) item. No need to add it to the pack since it's already there.
				return tempItem;
			}
		}
	} else if ((theItem->category & WEAPON) && theItem->quiverNumber > 0) {
		for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
			if (theItem->category == tempItem->category && theItem->kind == tempItem->kind
				&& theItem->quiverNumber == tempItem->quiverNumber) {				
				// We found a match!
                stackItems(tempItem, theItem);
				
				// Pass back the incremented (old) item. No need to add it to the pack since it's already there.
				return tempItem;
			}
		}
	}
	
	// assign a reference letter to the item
	itemLetter = nextAvailableInventoryCharacter();
	if (itemLetter) {
		theItem->inventoryLetter = itemLetter;
	}
	
	// insert at proper place in pack chain
	for (previousItem = packItems;
		 previousItem->nextItem != NULL && previousItem->nextItem->category <= theItem->category;
		 previousItem = previousItem->nextItem);
	theItem->nextItem = previousItem->nextItem;
	previousItem->nextItem = theItem;
	
	return theItem;
}

short numberOfItemsInPackWithoutAlchemy() {
	short theCount = 0;
	item *theItem;
	for(theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		theCount += (theItem->category & WEAPON) ? 1 : theItem->quantity;
	}
	return theCount;
}

short numberOfItemsInPack() {
	short theCount = 0;
	item *theItem;
	for(theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		theCount += ((theItem->category & WEAPON) || (rogue.talisman && rogue.talisman->kind == TALISMAN_ALCHEMY && (theItem->category & (POTION | SCROLL))) ? 1 : theItem->quantity);
	}
	return theCount;
}

char nextAvailableInventoryCharacter() {
	boolean charTaken[26];
	short i;
	item *theItem;
	char c;
	for(i=0; i<26; i++) {
		charTaken[i] = false;
	}
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		c = theItem->inventoryLetter;
		if (c >= 'a' && c <= 'z') {
			charTaken[c - 'a'] = true;
		}
	}
	for(i=0; i<26; i++) {
		if (!charTaken[i]) {
			return ('a' + i);
		}
	}
	return 0;
}

void updateFloorItems() {
    short x, y, loc[2];
    char buf[DCOLS*3], buf2[DCOLS*3];
    enum dungeonLayers layer;
    item *theItem, *nextItem;

	for (theItem=floorItems->nextItem; theItem != NULL; theItem = nextItem) {
		nextItem = theItem->nextItem;
		x = theItem->xLoc;
		y = theItem->yLoc;
		if (theItem->flags & ITEM_DORMANT) {
			if (!(--theItem->charges)) {
				theItem->flags &= ~(ITEM_NO_PICKUP | ITEM_PLAYER_AVOIDS | ITEM_DORMANT);
				if (theItem->flags & (ITEM_MAGIC_DETECTED)) {
					pmap[x][y].flags |= ITEM_DETECTED;
				}
				refreshDungeonCell(x, y);
			}
		}
		if ((cellHasTerrainFlag(x, y, T_IS_FIRE) && theItem->flags & ITEM_FLAMMABLE)
			|| (cellHasTerrainFlag(x, y, T_LAVA_INSTA_DEATH) && theItem->category != AMULET)) {
			burnItem(theItem);
			continue;
		}
		// Scrolls swirl in winds, potions float further than other items in water
		if  (!(cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) && (cellHasTerrainFlag(x, y, T_MOVES_ITEMS)
																	  || (theItem->category & SCROLL && cellHasTerrainFlag3(x, y, T3_IS_WIND))
																	  || (theItem->category & POTION && cellHasTerrainFlag(x, y, T_ALLOWS_SUBMERGING)))) {
			getQualifyingLocNear(loc, x, y, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
			
			removeItemFrom(x, y);
			pmap[loc[0]][loc[1]].flags |= HAS_ITEM;
			if (pmap[x][y].flags & ITEM_DETECTED) {
				pmap[x][y].flags &= ~ITEM_DETECTED;
				pmap[loc[0]][loc[1]].flags |= ITEM_DETECTED;
			}
			theItem->xLoc = loc[0];
			theItem->yLoc = loc[1];
			refreshDungeonCell(x, y);
			refreshDungeonCell(loc[0], loc[1]);
			continue;
		}
		if (cellHasTerrainFlag(x, y, T_AUTO_DESCENT)) {
			
			if (pmap[x][y].flags & VISIBLE) {
				itemName(theItem, buf, false, false, NULL);
				sprintf(buf2, "The %s plunge%s out of sight!", buf, (theItem->quantity > 1 ? "" : "s"));
				messageWithColor(buf2, &itemMessageColor, false);
			}
			theItem->flags |= ITEM_PREPLACED;
			
			// Remove from item chain.
			removeItemFromChain(theItem, floorItems);
			
			pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);
			
			if (theItem->category == POTION) {
				// Potions don't survive the fall.
				deleteItem(theItem);
			} else {
				// Add to next level's chain.
				theItem->nextItem = levels[rogue.depthLevel-1 + 1].items;
				levels[rogue.depthLevel-1 + 1].items = theItem;
			}
			refreshDungeonCell(x, y);
			continue;
		}
		if (cellHasTerrainFlag(x, y, T_PROMOTES_ON_STEP)) {
			for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
				if (tileCatalog[pmap[x][y].layers[layer]].flags & T_PROMOTES_ON_STEP) {
					promoteTile(x, y, layer, false);
				}
			}
			continue;
		}
        if (pmap[x][y].machineNumber
            && pmap[x][y].machineNumber == pmap[player.xLoc][player.yLoc].machineNumber
            && (theItem->flags & ITEM_KIND_AUTO_ID)) {
            
            identifyItemKind(theItem);
        }
	}	
}

boolean inscribeItem(item *theItem) {
	char itemText[30], buf[COLS], nameOfItem[COLS], oldInscription[COLS];
	
	strcpy(oldInscription, theItem->inscription);
	theItem->inscription[0] = '\0';
	itemName(theItem, nameOfItem, true, true, NULL);
	strcpy(theItem->inscription, oldInscription);
	
	sprintf(buf, "inscribe: %s \"", nameOfItem);
	if (getInputTextString(itemText, buf, 29, "", "\"", TEXT_INPUT_NORMAL, false)) {
		strcpy(theItem->inscription, itemText);
		confirmMessages();
		itemName(theItem, nameOfItem, true, true, NULL);
		nameOfItem[strlen(nameOfItem) - 1] = '\0';
		sprintf(buf, "%s %s.\"", (theItem->quantity > 1 ? "they're" : "it's"), nameOfItem);
		messageWithColor(buf, &itemMessageColor, false);
		return true;
	} else {
		confirmMessages();
		return false;
	}
}

boolean itemCanBeCalled(item *theItem) {
	
	updateIdentifiableItem(theItem); // Just in case.
	
	if ((theItem->flags & ITEM_IDENTIFIED) || theItem->category & (WEAPON|ARMOR|SHIELD|CHARM|FOOD|GOLD|AMULET|GEM)) {
		if (theItem->category & (WEAPON | ARMOR | SHIELD | STAFF | WAND | RING | CHARM | TALISMAN)
			&& (theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
			
			return true;
		} else {
			return false;
		}
	}
	if ((theItem->category & (POTION|ELIXIR|SCROLL|TOME|WAND|STAFF|RING|TALISMAN))
		&& !tableForItemCategory(theItem->category)->identified) {
		return true;
	}
	return false;
}

void call(item *theItem) {
	char itemText[30], buf[COLS];
	short c;
	unsigned char command[100];
	
	c = 0;
	command[c++] = CALL_KEY;
	if (theItem == NULL) {
		theItem = promptForItemOfType((WEAPON|ARMOR|SHIELD|SCROLL|TOME|RING|POTION|ELIXIR|STAFF|WAND|TALISMAN), 0, 0,
									  "Call what? (a-z, shift for more info; or <esc> to cancel)", true);
	}
	if (theItem == NULL) {
		return;
	}
	
	command[c++] = theItem->inventoryLetter;
	
	confirmMessages();
	
	if ((theItem->flags & ITEM_IDENTIFIED) || theItem->category & (WEAPON|ARMOR|SHIELD|FOOD|GOLD|AMULET|GEM)) {
		if (theItem->category & (WEAPON | ARMOR | SHIELD | STAFF | WAND | RING | CHARM | TALISMAN)
			&& (theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
			
			if (inscribeItem(theItem)) {
				command[c++] = '\0';
				strcat((char *) command, theItem->inscription);
				recordKeystrokeSequence(command);
				recordKeystroke(RETURN_KEY, false, false);
			}
		} else {
			message("you already know what that is.", false);
		}
		return;
	}
	
	if ((theItem->flags & ITEM_CAN_BE_IDENTIFIED)
		&& (theItem->category & (WEAPON | ARMOR | SHIELD | STAFF | WAND | RING | TALISMAN))) {
		if (confirm("Inscribe this particular item instead of all similar items?", true)) {
			command[c++] = 'y';
			if (inscribeItem(theItem)) {
				command[c++] = '\0';
				strcat((char *) command, theItem->inscription);
				recordKeystrokeSequence(command);
				recordKeystroke(RETURN_KEY, false, false);
			}
			return;
		} else {
			command[c++] = 'n'; // n means no
		}
	}
	
	if (getInputTextString(itemText, "call them: \"", 29, "", "\"", TEXT_INPUT_NORMAL, false)) {
		command[c++] = '\0';
		strcat((char *) command, itemText);
		recordKeystrokeSequence(command);
		recordKeystroke(RETURN_KEY, false, false);
		if (itemText[0]) {
			strcpy(tableForItemCategory(theItem->category)[theItem->kind].callTitle, itemText);
			tableForItemCategory(theItem->category)[theItem->kind].called = true;
		} else {
			tableForItemCategory(theItem->category)[theItem->kind].callTitle[0] = '\0';
			tableForItemCategory(theItem->category)[theItem->kind].called = false;
		}
		confirmMessages();
		itemName(theItem, buf, false, true, NULL);
		messageWithColor(buf, &itemMessageColor, false);
	} else {
		confirmMessages();
	}
}

// Generates the item name and returns it in the "root" string.
// IncludeDetails governs things such as enchantment, charges, strength requirement, times used, etc.
// IncludeArticle governs the article -- e.g. "some" food, "5" darts, "a" pink potion.
// If baseColor is provided, then the suffix will be in gray, flavor portions of the item name (e.g. a "pink" potion,
//	a "sandalwood" staff, a "ruby" ring) will be in dark purple, and the Amulet of Yendor and lumenstones will be in yellow.
//  BaseColor itself will be the color that the name reverts to outside of these colored portions.
void itemName(item *theItem, char *root, boolean includeDetails, boolean includeArticle, color *baseColor) {
	char buf[3*DCOLS], pluralization[10], article[10] = "",
	grayEscapeSequence[5], purpleEscapeSequence[5], yellowEscapeSequence[5], baseEscapeSequence[5];
	color tempColor;
	short i;
	
	strcpy(pluralization, (theItem->quantity > 1 ? "s" : ""));
	
	grayEscapeSequence[0] = '\0';
	purpleEscapeSequence[0] = '\0';
	yellowEscapeSequence[0] = '\0';
	baseEscapeSequence[0] = '\0';
	if (baseColor) {
		tempColor = backgroundMessageColor;
		applyColorMultiplier(&tempColor, baseColor); // To gray out the purle if necessary.
		encodeMessageColor(purpleEscapeSequence, 0, &tempColor);
		
		tempColor = gray;
		//applyColorMultiplier(&tempColor, baseColor);
		encodeMessageColor(grayEscapeSequence, 0, &tempColor);
        
		tempColor = itemMessageColor;
		applyColorMultiplier(&tempColor, baseColor);
		encodeMessageColor(yellowEscapeSequence, 0, &tempColor);
		
		encodeMessageColor(baseEscapeSequence, 0, baseColor);
	}
	
	switch (theItem -> category) {
		case FOOD:
			if (theItem -> kind == FRUIT) {
				sprintf(root, "mango%s", pluralization);
			} else {
				if (theItem->quantity == 1) {
					sprintf(article, "some ");
					sprintf(root, "food");
				} else {
					sprintf(root, "ration%s of food", pluralization);
				}
			}
			break;
		case WEAPON:
			sprintf(root, "%s%s", weaponTable[theItem->kind].name, pluralization);
			if (includeDetails) {
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
					strcpy(root, buf);
				}
				
				if (itemRunic(theItem)) {
					if (itemRunicKnown(theItem) || rogue.playbackOmniscience) {
						if (itemRune(theItem) == W_SLAYING) {
							sprintf(buf, "%s of %s slaying%s",
									root,
									monsterCatalog[theItem->vorpalEnemy].monsterName,
									grayEscapeSequence);
						} else {
							sprintf(buf, "%s of %s%s",
									root,
									weaponRunicNames[itemRune(theItem)],
									grayEscapeSequence);
						}
						strcpy(root, buf);
					} else if (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) {
						if (grayEscapeSequence[0]) {
							strcat(root, grayEscapeSequence);
						}
						strcat(root, " (unknown runic)");
					}
				} else if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY) {
					if (itemRunicKnown(theItem) || rogue.playbackOmniscience) {
						if (grayEscapeSequence[0]) {
							strcat(root, grayEscapeSequence);
						}
						if (itemRune(theItem) == W_SLAYING) {
							sprintf(buf, "%s (%i enchant%s for %s slaying)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									monsterCatalog[theItem->vorpalEnemy].monsterName);
						} else {
							sprintf(buf, "%s (%i enchant%s for %s)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									weaponRunicNames[itemRune(theItem)]);
						}
						strcpy(root, buf);
					} else if (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) {
						if (grayEscapeSequence[0]) {
							strcat(root, grayEscapeSequence);
						}
						if (theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s to make runic)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "");
							strcpy(root, buf);
						} else {
							strcat(root, " (unknown runic)");
						}
					}
				}
				sprintf(buf, "%s%s <%i>", root, grayEscapeSequence, theItem->strengthRequired);
				strcpy(root, buf);
			}
			break;
		case ARMOR:
			sprintf(root, "%s", armorTable[theItem->kind].name);
			if (includeDetails) {
				
				if (itemRunic(theItem) 
					&& (itemRunicKnown(theItem)
						|| rogue.playbackOmniscience)) {
					if (theItem->enchant2 == A_IMMUNITY) {
						sprintf(buf, "%s of %s immunity", root, monsterCatalog[theItem->vorpalEnemy].monsterName);
					} else {
						sprintf(buf, "%s of %s", root, armorRunicNames[itemRune(theItem)]);
					}
					strcpy(root, buf);
				}
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					if (theItem->enchant1 == 0) {
						sprintf(buf, "%s%s [%i]<%i>", root, grayEscapeSequence, theItem->armor/10, theItem->strengthRequired);
					} else {
						sprintf(buf, "%s%i %s%s [%i]<%i>",
								(theItem->enchant1 < 0 ? "" : "+"),
								theItem->enchant1,
								root,
								grayEscapeSequence,
								theItem->armor/10 + theItem->enchant1,
								theItem->strengthRequired);
					}
				} else {
					sprintf(buf, "%s%s <%i>", root, grayEscapeSequence, theItem->strengthRequired);
				}
				strcpy(root, buf);
				
				if (itemRunic(theItem)
					&& (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED))
					&& !(itemRunicKnown(theItem))
					&& !rogue.playbackOmniscience) {
					strcat(root, " (unknown runic)");
				} else if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY) {
					if (itemRunicKnown(theItem)
							   || rogue.playbackOmniscience) {
						if (itemRune(theItem) == A_IMMUNITY && theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s for %s immunity)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									monsterCatalog[theItem->vorpalEnemy].monsterName);
							strcpy(root, buf);
						} else if (theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s for %s)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									armorRunicNames[itemRune(theItem)]);
							strcpy(root, buf);
						}
					} else if (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) {
						if (theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s to make runic)",
								root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "");
							strcpy(root, buf);
						} else {
							strcat(root, " (unknown runic)");
						}
					}
				}
			}
			break;
		case SHIELD: // uses armor runics
			sprintf(root, "%s", shieldTable[theItem->kind].name);
			if (includeDetails) {
				
				if (itemRunic(theItem)
					&& (itemRunicKnown(theItem)
						|| rogue.playbackOmniscience)) {
					if (theItem->enchant2 == A_IMMUNITY) {
						sprintf(buf, "%s of %s immunity", root, monsterCatalog[theItem->vorpalEnemy].monsterName);
					} else {
						sprintf(buf, "%s of %s", root, armorRunicNames[itemRune(theItem)]);
					}
					strcpy(root, buf);
				}
				
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					if (theItem->enchant1 == 0) {
						sprintf(buf, "%s%s (%i)<%i>", root, grayEscapeSequence, theItem->shieldBlows, theItem->strengthRequired);
					} else {
						sprintf(buf, "%s%i %s%s (%i)<%i>",
								(theItem->enchant1 < 0 ? "" : "+"),
								theItem->enchant1,
								root,
								grayEscapeSequence,
								theItem->shieldBlows,
								theItem->strengthRequired);
					}
					strcpy(root, buf);
				} else {
					sprintf(root, "%s%s <%i>", root, grayEscapeSequence, theItem->strengthRequired);
				}
				
				if (itemRunic(theItem)
					&& (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED))
					&& !(itemRunicKnown(theItem))
					&& !rogue.playbackOmniscience) {
					strcat(root, " (unknown runic)");
				} else if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY) {
					if (itemRunicKnown(theItem)
						|| rogue.playbackOmniscience) {
						if (itemRune(theItem) == A_IMMUNITY && theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s for %s immunity)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									monsterCatalog[theItem->vorpalEnemy].monsterName);
							strcpy(root, buf);
						} else if (theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s for %s)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "",
									armorRunicNames[itemRune(theItem)]);
							strcpy(root, buf);
						}
					} else if ((theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED))) {
						if (theItem->hiddenRunicEnchantsRequired > 0) {
							sprintf(buf, "%s (%i enchant%s to make runic)",
									root, theItem->hiddenRunicEnchantsRequired, theItem->hiddenRunicEnchantsRequired > 1 ? "s" : "");
							strcpy(root, buf);
						} else {
							strcat(root, " (unknown runic)");
						}
					}
				}
			}
			break;
		case SCROLL:
			if (scrollTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "scroll%s of %s", pluralization, scrollTable[theItem->kind].name);
			} else if (scrollTable[theItem->kind].called) {
				sprintf(root, "scroll%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						scrollTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "scroll%s entitled %s\"%s\"%s",
						pluralization,
						purpleEscapeSequence,
						scrollTable[theItem->kind].flavor,
						baseEscapeSequence);
			}
			break;
		case TOME:
			if (tomeTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "tome%s of %s", pluralization, tomeTable[theItem->kind].name[0] ? tomeTable[theItem->kind].name : scrollTable[theItem->kind].name);
			} else if (tomeTable[theItem->kind].called) {
				sprintf(root, "tome%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						tomeTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "tome%s entitled %s\"%s\"%s",
						pluralization,
						purpleEscapeSequence,
						tomeTable[theItem->kind].flavor,
						baseEscapeSequence);
			}
			break;
		case POTION:
			if (potionTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "potion%s of %s", pluralization, potionTable[theItem->kind].name);
			} else if (potionTable[theItem->kind].called) {
				sprintf(root, "potion%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						potionTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s potion%s",
						purpleEscapeSequence,
						potionTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			break;
		case ELIXIR:
			if (elixirTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "elixir%s of %s", pluralization, elixirTable[theItem->kind].name[0] ? elixirTable[theItem->kind].name : potionTable[theItem->kind].name);
			} else if (elixirTable[theItem->kind].called) {
				sprintf(root, "elixir%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						elixirTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s elixir%s",
						purpleEscapeSequence,
						elixirTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			break;
		case WAND:
			if (wandTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "wand%s of %s",
						pluralization,
						wandTable[theItem->kind].name[0] ? wandTable[theItem->kind].name : boltTable[theItem->kind].name);
			} else if (wandTable[theItem->kind].called) {
				sprintf(root, "wand%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						wandTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s wand%s",
						purpleEscapeSequence,
						wandTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			if (includeDetails) {
				if (theItem->numberOfBoltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
					if (theItem->flags & ITEM_RUNIC_IDENTIFIED || rogue.playbackOmniscience) {
						for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
							sprintf(buf, "%s, %s",
									root,
									boltRunicCatalog[theItem->boltEnchants[i]].name);
							strcpy(root, buf);
						}
					}
				}
				if (theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN) || rogue.playbackOmniscience) {
					sprintf(buf, "%s%s [%i]",
							root,
							grayEscapeSequence,
							theItem->charges);
					strcpy(root, buf);
				} else if (theItem->enchant2 > 2) {
					sprintf(buf, "%s%s (used %i times)",
							root,
							grayEscapeSequence,
							theItem->enchant2);
					strcpy(root, buf);
				} else if (theItem->enchant2) {
					sprintf(buf, "%s%s (used %s)",
							root,
							grayEscapeSequence,
							(theItem->enchant2 == 2 ? "twice" : "once"));
					strcpy(root, buf);
				}
				if (theItem->numberOfBoltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
					if ((theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) && !(theItem->flags & ITEM_RUNIC_IDENTIFIED) && !rogue.playbackOmniscience) {
						if (grayEscapeSequence[0]) {
							strcat(root, grayEscapeSequence);
						}						
						sprintf(buf, "%s (unknown runic%s)",
								root,
								theItem->numberOfBoltEnchants > ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1) ? "s" : "");
						strcpy(root, buf);
					}
				}				
			}
			break;
		case STAFF:
			if (staffTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "staff%s of %s",
						pluralization,
						staffTable[theItem->kind].name[0] ? staffTable[theItem->kind].name : boltTable[theItem->kind].name);
			} else if (staffTable[theItem->kind].called) {
				sprintf(root, "staff%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						staffTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s staff%s",
						purpleEscapeSequence,
						staffTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			if (includeDetails) {
				if (theItem->numberOfBoltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
					if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
						for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
							sprintf(buf, "%s, %s",
									root,
									boltRunicCatalog[theItem->boltEnchants[i]].name);
							strcpy(root, buf);
						}
					}
				}
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					sprintf(buf, "%s%s [%i/%i]", root, grayEscapeSequence, theItem->charges, theItem->enchant1);
					strcpy(root, buf);
				} else if (theItem->flags & ITEM_MAX_CHARGES_KNOWN) {
					sprintf(buf, "%s%s [?/%i]", root, grayEscapeSequence, theItem->enchant1);
					strcpy(root, buf);
				}
				if (theItem->numberOfBoltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
					if ((theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) && !(theItem->flags & ITEM_RUNIC_IDENTIFIED) && !rogue.playbackOmniscience) {
						if (grayEscapeSequence[0]) {
							strcat(root, grayEscapeSequence);
						}
						sprintf(buf, "%s (unknown runic%s)",
								root,
								theItem->numberOfBoltEnchants > ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1) ? "s" : "");
						strcpy(root, buf);
					}
				}
			}
			break;
		case RING:
			if (ringTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "ring%s of %s", pluralization, ringTable[theItem->kind].name);
			} else if (ringTable[theItem->kind].called) {
				sprintf(root, "ring%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						ringTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s ring%s",
						purpleEscapeSequence,
						ringTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			if (includeDetails && ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)) {
				sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
				strcpy(root, buf);
			}
			break;
		case CHARM:
            sprintf(root, "%s charm%s", charmTable[theItem->kind].name, pluralization);
            
			if (includeDetails) {
				sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
				strcpy(root, buf);
                
                if (theItem->charges) {
                    sprintf(buf, "%s %s(%i%%)",
                            root,
                            grayEscapeSequence,
                            (charmRechargeDelay(theItem->kind, theItem->enchant1) - theItem->charges) * 100 / charmRechargeDelay(theItem->kind, theItem->enchant1)) + (theItem->flags & ITEM_OVERCHARGED ? 100 : 0);
                    strcpy(root, buf);
                } else {
                    strcat(root, grayEscapeSequence);
                    strcat(root, " (ready)");
                }
			}
			break;
		case TALISMAN:
			if (talismanTable[theItem->kind].identified || rogue.playbackOmniscience) {
				sprintf(root, "talisman%s of %s", pluralization, talismanTable[theItem->kind].name);
			} else if (talismanTable[theItem->kind].called) {
				sprintf(root, "talisman%s called %s%s%s",
						pluralization,
						purpleEscapeSequence,
						talismanTable[theItem->kind].callTitle,
						baseEscapeSequence);
			} else {
				sprintf(root, "%s%s%s talisman%s",
						purpleEscapeSequence,
						talismanTable[theItem->kind].flavor,
						baseEscapeSequence,
						pluralization);
			}
			if (includeDetails && ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) && theItem->kind < TALISMAN_MAX_ENCHANT) {
				sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
				strcpy(root, buf);
			}
			break;
		case GOLD:
			sprintf(root, "gold piece%s", pluralization);
			break;
		case AMULET:
			sprintf(root, "%sAmulet%s of Yendor%s", yellowEscapeSequence, pluralization, baseEscapeSequence);
			break;
		case GEM:
			sprintf(root, "%slumenstone%s%s", yellowEscapeSequence, pluralization, baseEscapeSequence);
			break;
		case KEY:
			if (includeDetails && theItem->keyZ > 0 && theItem->keyZ != rogue.depthLevel) {
				sprintf(root, "%s%s%s from depth %i",
						keyTable[theItem->kind].name,
						pluralization,
						grayEscapeSequence,
						theItem->keyZ);
			} else if (includeDetails && theItem->kind == KEY_RUNE_ARMOR) {
				if (theItem->enchant2 == A_IMMUNITY) {
					sprintf(root, "%s%s of %s immunity", keyTable[theItem->kind].name,
							pluralization,
							monsterCatalog[theItem->vorpalEnemy].monsterName);
				} else {
					sprintf(root, "%s%s of %s", keyTable[theItem->kind].name,
							pluralization,
							armorRunicNames[theItem->enchant2]);
				}
			} else if (includeDetails && theItem->kind == KEY_RUNE_WEAPON) {
				if (theItem->enchant2 == W_SLAYING) {
					sprintf(root, "%s%s of %s slaying", keyTable[theItem->kind].name,
							pluralization,
							monsterCatalog[theItem->vorpalEnemy].monsterName);
				} else {
					sprintf(root, "%s%s of %s", keyTable[theItem->kind].name,
							pluralization,
							weaponRunicNames[theItem->enchant2]);
				}
			} else if (includeDetails && (theItem->kind == KEY_RUNE_WAND || theItem->kind == KEY_RUNE_STAFF)) {
				sprintf(root, "%s %s%s",
						boltRunicCatalog[theItem->enchant2].name,
						keyTable[theItem->kind].name,
						pluralization);
			} else {
				sprintf(root,
						keyTable[theItem->kind].name,
						"%s%s",
						pluralization);
			}
			break;
		default:
			sprintf(root, "unknown item%s", pluralization);
			break;
	}
	
	if (includeArticle) {
		// prepend number if quantity is over 1
		if (theItem->quantity > 1) {
			sprintf(article, "%i ", theItem->quantity);
		} else if (theItem->category & AMULET) {
			sprintf(article, "the ");
		} else if (!(theItem->category & ARMOR) && !((theItem->category & FOOD) && theItem->kind == RATION)) {
			// otherwise prepend a/an if the item is not armor and not a ration of food;
			// armor gets no article, and "some food" was taken care of above.
			sprintf(article, "a%s ", (isVowel(root) ? "n" : ""));
		}
	}
	// strcat(buf, suffixID);
	if (includeArticle) {
		sprintf(buf, "%s%s", article, root);
		strcpy(root, buf);
	}
	
	if (includeDetails && theItem->inscription[0]) {
		if (theItem->flags & ITEM_CAN_BE_IDENTIFIED) {
			sprintf(buf, "%s \"%s\"", root, theItem->inscription);
			strcpy(root, buf);
		} else {
			theItem->inscription[0] = '\0';
		}
	}
	return;
}

itemTable *tableForItemCategory(enum itemCategory theCat) {
	switch (theCat) {
		case FOOD:
			return foodTable;
		case WEAPON:
			return weaponTable;
		case ARMOR:
			return armorTable;
		case SHIELD:
			return shieldTable;
		case POTION:
			return potionTable;
		case ELIXIR:
			return elixirTable;
		case SCROLL:
			return scrollTable;
		case TOME:
			return tomeTable;
		case RING:
			return ringTable;
		case WAND:
			return wandTable;
		case STAFF:
			return staffTable;
		case CHARM:
			return charmTable;
		case TALISMAN:
			return talismanTable;
		case SUMMONING_STAFF:
			return summoningStaffTable;
		default:
			return NULL;
	}
}

itemTable *inheritedTableForItemCategory(enum itemCategory theCat) {
	switch (theCat) {
		case ELIXIR:
			return potionTable;
		case TOME:
			return scrollTable;
		case WAND:
		case STAFF:
			return boltTable;
		default:
			return NULL;
	}
}



boolean isVowel(char *theChar) {
	while (*theChar == COLOR_ESCAPE) {
		theChar += 4;
	}
	return (*theChar == 'a' || *theChar == 'e' || *theChar == 'i' || *theChar == 'o' || *theChar == 'u' ||
			*theChar == 'A' || *theChar == 'E' || *theChar == 'I' || *theChar == 'O' || *theChar == 'U');
}

short charmEffectDuration(short charmKind, short enchant) {
    const short duration[NUMBER_CHARM_KINDS] = {
        3,  // Health
		100, // Strength
        25, // Telepathy
        10, // Levitation
		0,	// Detect Magic
        7,  // Haste
        10, // Fire immunity
        5,  // Invisibility
		0,	// Winds
		0,	// Poisonous gas
		0,	// Paralysis
		50,	// Hallucination
		0,	// Confusion
		0,	// Incineration
		0,	// Darkness
		0,	// Descent
		0,	// Creeping death
		0,	// Water
		0,	// Stench
		0,	// Explosive gas
		0,	// Enchanting
		0,	// Identify
        0,  // Teleportation
        0,  // Recharging
        20, // Protection
		20,	// Protection
		0,	// Magic mapping
        0,  // Cause fear
        0,  // Negation
        0,  // Shattering
        0,  // Duplication
        20,  // Aggravate monsters
        10,  // Summon monsters
    };
    const short increment[NUMBER_CHARM_KINDS] = {
        0,  // Health
		0,  // Strength
        25, // Telepathy
        25, // Levitation
		0,	// Detect magic
        20, // Haste
        25, // Fire immunity
        20, // Invisibility
		0,	// Winds
		0,	// Poisonous gas
		0,	// Paralysis
		0,	// Hallucination
		0,	// Confusion
		0,	// Incineration
		0,	// Darkness
		0,	// Descent
		0,	// Creeping death
		0,	// Water
		0,	// Stench
		0,	// Explosive gas
		0,	// Enchanting
		0,	// Identify
        0,  // Teleportation
        0,  // Recharging
        0,	// Protection
		0,	// Protection
		0,	// Magic mapping
        0,  // Cause fear
        0,  // Negation
        0,  // Shattering
        0,  // Duplication
        10,  // Aggravate monsters
        5,  // Summon monsters
    };
    
    return duration[charmKind] * pow((double) (100 + (increment[charmKind])) / 100, enchant);
}

short charmRechargeDelay(short charmKind, short enchant) {
    const short duration[NUMBER_CHARM_KINDS] = {
        2500,   // Health
		800,	// Strength
        800,    // Telepathy
        800,    // Levitation
        5000,	// Detect Magic
        800,    // Haste
        800,    // Fire immunity
        800,    // Invisibility
		400,	// Winds
		600,	// Poisonous gas
		600,	// Paralysis
		400,	// Hallucination
		400,	// Confusion
		400,	// Incineration
		400,	// Darkness
		800,	// Descent
		400,	// Creeping death
		800,	// Water
		400,	// Stench
		3000,	// Explosive gas
		10000,	// Enchanting
		5000,	// Identify
        1000,   // Teleportation
        10000,  // Recharging
        1000,   // Protection
        0,		// Unused
		3000,	// Magic mapping
        3000,   // Cause fear
        2500,   // Negation
        2500,   // Shattering
        10000,	// Duplication
        400,	// Aggravate monsters
        3000,	// Summon monsters
    };
    const short increment[NUMBER_CHARM_KINDS] = {
		//        35, // Health
		//        30, // Protection
		//        25, // Haste
		//        25, // Fire immunity
		//        20, // Invisibility
		//        30, // Telepathy
		//        25, // Levitation
		//        40, // Shattering
		//        20, // Cause fear
		//        20, // Teleportation
		//        30, // Recharging
		//        25, // Negation
        45, // Health
		35,	// Strength
        35, // Telepathy
        35, // Levitation
        45, // Detect Magic
        35, // Haste
        40, // Fire immunity
        35, // Invisibility
		20,	// Winds
		25,	// Poisonous gas
		25,	// Paralysis
		20,	// Hallucination
		20,	// Confusion
		20,	// Incineration
		20,	// Darkness
		25,	// Descent
		20,	// Creeping death
		25,	// Water
		20,	// Stench
		25,	// Explosive gas
		100,// Enchanting
		45,	// Identify
        45, // Teleportation
        40, // Recharging
        40, // Protection
		0,	// Unused
		45,	// Magic mapping
        35, // Cause fear
        40, // Negation
        40, // Shattering
        40,	// Duplication
        20,	// Aggravate monsters
        35,	// Summon monsters
    };
    
    return charmEffectDuration(charmKind, enchant) + duration[charmKind] * pow((double) (100 - (increment[charmKind])) / 100, enchant);
}

float enchantIncrement(item *theItem) {
	if (theItem->category & (WEAPON | ARMOR | SHIELD)) {
		if (theItem->strengthRequired == 0) {
			return 1 + 0;
		} else if (theItem->category & (SHIELD)) {
			if (rogue.strength - player.weaknessAmount + rogue.mightBonus < theItem->strengthRequired) {
				return 1 + 2.5;
			} else {
				return 1 + 0.25;
			}
		} else if (rogue.strength - player.weaknessAmount + rogue.mightBonus
				   - (rogue.shield && !(theItem->category & (SHIELD)) ? rogue.shield->kind * 2 + 1: 0)
				   - (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0)
				   < theItem->strengthRequired) {
			return 1 + 2.5;
		} else {
			return 1 + 0.25;
		}
	} else {
		return 1 + 0;
	}
}

// Items:
// $WANDORSTAFF: wand or staff, this wand or this staff
void resolveItemEscapes(char *text, item *theItem, boolean addThis, boolean plural) {
	short escapeType, i;
	char *insert, *scan;
	boolean capitalize;
	int offset;
	unsigned long category = theItem->category;
	// Note: Escape sequences MUST be longer than EACH of the possible replacements.
	// That way, the string only contracts, and we don't need a buffer.
	const char escapes[5][9][20] = {
		{"$WANDORSTAFF", "wand", "staff", "wands", "staffs", "this wand", "this staff", "these wands", "these staffs"},
		{"$WOODORMETAL", "metal", "wood", "metal", "wood", "this metal", "this wood", "this metal", "this wood"},
		{"$POTIONORELIXIR", "potion", "elixirs", "potions", "elixirs", "this potion", "this elixir", "these potions", "these elixirs"},
		{"$SCROLLORTOME", "scroll", "tome", "scrolls", "tomes", "this scroll", "this tome", "these scrolls", "these tomes"},
		{"$PARCHMENTORPAGES", "this parchment", "these pages", "these parchments", "these pages", "these parchments", "these pages"} // this/these is always used
	};
	const unsigned long escapeCategories[5] = {
		WAND,
		WAND,
		POTION,
		SCROLL,
		SCROLL
	};

	capitalize = false;	
	if (category == KEY && theItem->kind == KEY_RUNE_WAND) {
		category = WAND;
	}

	if (addThis && plural) {
		offset = 7;
	} else if (plural) {
		offset = 3;
	} else if (addThis) {
		offset = 5;
	} else {
		offset = 1;
	}							
	
	for (insert = scan = text; *scan;) {
		if (scan[0] == '$') {
			for (escapeType=0; escapeType < 5; escapeType++) {
				if (stringsMatch(escapes[escapeType][0], scan)) {
					if (category & escapeCategories[escapeType]) {
						strcpy(insert, escapes[escapeType][offset]);
					} else {
						strcpy(insert, escapes[escapeType][offset+1]);
					}
					if (capitalize) {
						upperCase(insert);
						capitalize = false;
					}
					scan += strlen(escapes[escapeType][0]);
					if (category & escapeCategories[escapeType]) {
						insert += strlen(escapes[escapeType][offset]);
					} else {
						insert += strlen(escapes[escapeType][offset+1]);
					}
					break;
				}
			}
			if (escapeType == 5) {
				// Started with a '$' but didn't match an escape sequence; just copy the character and move on.
				*(insert++) = *(scan++);
			}
		} else if (scan[0] == COLOR_ESCAPE) {
			for (i=0; i<4; i++) {
				*(insert++) = *(scan++);
			}
		} else { // Didn't match any of the escape sequences; copy the character instead.
			if (*scan == '.') {
				capitalize = true;
			} else if (*scan != ' ') {
				capitalize = false;
			}
			
			*(insert++) = *(scan++);
		}
	}
	*insert = '\0';
}


boolean itemIsCarried(item *theItem) {
	item *tempItem;
	
	for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
		if (tempItem == theItem) {
			return true;
		}
	}
	return false;
}

void itemDetails(char *buf, item *theItem) {
	char buf2[1000], buf3[1000], theName[100], theName2[100], goodColorEscape[20], badColorEscape[20], whiteColorEscape[20];
	boolean singular, carried;
	float enchant;
	short nextLevelState = 0, noise, i, n, numberOfBolts;
	unsigned long boltHasFlags;
	float accuracyChange, damageChange, current, new, currentDamage, newDamage, blockRange, boltPower;
	const char weaponRunicEffectDescriptions[NUMBER_WEAPON_RUNIC_KINDS][DCOLS] = {
		"time will stop while you take an extra turn",
		"the enemy will die instantly",
		"the enemy will be paralyzed",
		"[multiplicity]", // never used
		"the enemy will be slowed",
		"the enemy will be confused",
		"the enemy will be flung",
		"[slaying]", // never used
		"the enemy will be healed",
		"the enemy will be cloned",
	};
	
    goodColorEscape[0] = badColorEscape[0] = whiteColorEscape[0] = '\0';
    encodeMessageColor(goodColorEscape, 0, &goodMessageColor);
    encodeMessageColor(badColorEscape, 0, &badMessageColor);
    encodeMessageColor(whiteColorEscape, 0, &white);
	
	singular = (theItem->quantity == 1 ? true : false);
	carried = itemIsCarried(theItem);
	
	// Name
	itemName(theItem, theName, true, true, NULL);
	buf[0] = '\0';
	encodeMessageColor(buf, 0, &itemMessageColor);
	upperCase(theName);
	strcat(buf, theName);
	if (carried) {
		sprintf(buf2, " (%c)", theItem->inventoryLetter);
		strcat(buf, buf2);
	}
	encodeMessageColor(buf, strlen(buf), &white);
	strcat(buf, "\n\n");
	
	itemName(theItem, theName, false, false, NULL);
	
	// introductory text
	if (tableForItemCategory(theItem->category)
		&& (tableForItemCategory(theItem->category)[theItem->kind].identified || rogue.playbackOmniscience)) {
		
		// Elixirs, tomes, staffs, wands prepend the base item description if blank or starting with a space
		if (inheritedTableForItemCategory(theItem->category) &&
			(tableForItemCategory(theItem->category)[theItem->kind].description[0] == ' ' ||
			tableForItemCategory(theItem->category)[theItem->kind].description[0] == '\0')) {
			strcat(buf, inheritedTableForItemCategory(theItem->category)[theItem->kind].description);
		}
		
		strcat(buf, tableForItemCategory(theItem->category)[theItem->kind].description);
		resolveItemEscapes(buf, theItem, false, false);
		
		if (theItem->category == POTION && theItem->kind == POTION_LIFE) {
            sprintf(buf2, "\n\nIt will increase your maximum health by %s%i%%%s.",
                    goodColorEscape,
                    (player.info.maxHP + 10) * 100 / player.info.maxHP - 100,
                    whiteColorEscape);
            strcat(buf, buf2);
        }		
	} else {
		switch (theItem->category) {
			case POTION:
			case ELIXIR:
				sprintf(buf2, "%s %sflask%s contain%s a %s %s liquid. \
Who knows what %s will do when drunk or thrown?",
						(singular ? "This" : "These"),
						(theItem->category & ELIXIR ? "narrow " : ""),
						(singular ? "" : "s"),
						(singular ? "s" : ""),
						elixirTable[theItem->kind].flavor,
						potionTable[theItem->kind].flavor,
						(singular ? "it" : "they"));
				break;
			case TOME:
				sprintf(buf2, "%s cover%s %s is stained with strange inks, and bear%s a title of \"%s.\" \
Who knows what %s will do when read aloud?",
						(singular ? "This" : "These"),
						(singular ? "" : "s"),
						(singular ? "is" : "are"),
						(singular ? "s" : ""),
						tableForItemCategory(theItem->category)[theItem->kind].flavor,
						(singular ? "it" : "they"));
				break;
			case SCROLL:
				sprintf(buf2, "%s parchment%s %s covered with indecipherable writing, and bear%s a title of \"%s.\" \
Who knows what %s will do when read aloud?",
						(singular ? "This" : "These"),
						(singular ? "" : "s"),
						(singular ? "is" : "are"),
						(singular ? "s" : ""),
						tableForItemCategory(theItem->category)[theItem->kind].flavor,
						(singular ? "it" : "they"));
				break;
			case STAFF:
				sprintf(buf2, "This gnarled %s staff is warm to the touch. \
Who knows what it will do when used?",
						tableForItemCategory(theItem->category)[theItem->kind].flavor);
				break;
			case WAND:
				sprintf(buf2, "This thin %s wand is warm to the touch. \
Who knows what it will do when used?",
						tableForItemCategory(theItem->category)[theItem->kind].flavor);
				break;
			case RING:
				sprintf(buf2, "This metal band is adorned with a large %s gem that glitters in the darkness. \
Who knows what effect it has when worn? ",
						tableForItemCategory(theItem->category)[theItem->kind].flavor);
				break;
			case CHARM: // Should never be displayed.
				strcat(buf2, "What a perplexing charm!");
				break;
			case TALISMAN:
				sprintf(buf2, "This %s object is imbued with powerful magic. \
Who knows what effect it has when worn? ",
						tableForItemCategory(theItem->category)[theItem->kind].flavor);
				break;
			case AMULET:
				strcpy(buf2, "Legends are told about this mysterious golden amulet, \
and hundreds of adventurers have perished in its pursuit. Unfathomable power and riches await anyone with the skill and ambition \
to carry it into the light of day.");
				break;
			case GEM:
				sprintf(buf2, "Mysterious lights swirl and fluoresce beneath the stone%s surface. \
Lumenstones are said to contain mysterious properties of untold power, but for you, they mean one thing: riches.",
						(singular ? "'s" : "s'"));
				break;
			case KEY:
				strcpy(buf2, keyTable[theItem->kind].description);
				break;
			case GOLD:
				sprintf(buf2, "A pile of %i shining gold coins.", theItem->quantity);
				break;
			default:
				break;
		}
		strcat(buf, buf2);
	}
	
	// detailed description
	switch (theItem->category) {
			
		case FOOD:
			sprintf(buf2, "\n\nYou are %shungry enough to fully enjoy a %s.",
					((STOMACH_SIZE - player.status[STATUS_NUTRITION]) >= foodTable[theItem->kind].strengthRequired ? "" : "not yet "),
					(theItem->kind == RATION ? "ration of food" : theName));
			strcat(buf, buf2);
			break;
		
		case WEAPON:
		case ARMOR:
		case SHIELD:
			// enchanted? strength modifier?
			if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
				if (theItem->enchant1) {
					sprintf(buf2, "\n\nThe %s bear%s an intrinsic %s%s%i%s",
							theName,
							(singular ? "s" : ""),
							(theItem->enchant1 > 0 ? "enchantment of +" : "penalty of "),
                            (theItem->enchant1 > 0 ? goodColorEscape : badColorEscape),
							theItem->enchant1,
                            whiteColorEscape);
				} else {
					sprintf(buf2, "\n\nThe %s bear%s no intrinsic enchantment",
							theName,
							(singular ? "s" : ""));
				}
				strcat(buf, buf2);
				if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY && theItem->hiddenRunicEnchantsRequired < 0) {
					sprintf(buf2, ", but %s %s penalty of%s%.2f%s because you enchanted it %s",
							(singular ? "carries" : "carry"),
							(theItem->enchant1 < 0 ? "an additional" : "a"),
                            badColorEscape,
							(float)theItem->hiddenRunicEnchantsRequired * 1.25,
							whiteColorEscape,
							(theItem->flags & ITEM_RUNIC ? "when it was naturally runic" : "after the rune was revealed"));
					strcat(buf, buf2);
				}
				if (strengthModifier(theItem)) {
					sprintf(buf2, ", %s %s %s %s%s%.2f%s because of your %s strength. ",
							(theItem->enchant1 || (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY && theItem->hiddenRunicEnchantsRequired < 0) ? "and" : "but"),
							(singular ? "carries" : "carry"),
							(theItem->enchant1 && (theItem->enchant1 > 0) == (strengthModifier(theItem) > 0) ? "an additional" : "a"),
							(strengthModifier(theItem) > 0 ? "bonus of +" : "penalty of "),
                            (strengthModifier(theItem) > 0 ? goodColorEscape : badColorEscape),
							strengthModifier(theItem),
							whiteColorEscape,
							(strengthModifier(theItem) > 0 ? "excess" : "inadequate"));
					strcat(buf, buf2);
				} else {
					strcat(buf, ". ");
				}
			} else {
				if ((theItem->enchant1 > 0) && (theItem->flags & ITEM_MAGIC_DETECTED)) {
					sprintf(buf2, "\n\nYou can feel an %saura of benevolent magic%s radiating from the %s. ",
                            goodColorEscape,
                            whiteColorEscape,
							theName);
					strcat(buf, buf2);
				}
				if (strengthModifier(theItem)) {
					sprintf(buf2, "\n\nThe %s %s%s a %s%s%.2f%s because of your %s strength. ",
							theName,
							((theItem->enchant1 > 0) && (theItem->flags & ITEM_MAGIC_DETECTED) ? "also " : ""),
							(singular ? "carries" : "carry"),
							(strengthModifier(theItem) > 0 ? "bonus of +" : "penalty of "),
                            (strengthModifier(theItem) > 0 ? goodColorEscape : badColorEscape),
							strengthModifier(theItem),
                            whiteColorEscape,
							(strengthModifier(theItem) > 0 ? "excess" : "inadequate"));
					strcat(buf, buf2);
				}
				
				if (theItem->category & WEAPON) {
					sprintf(buf2, "It will reveal its secrets to you if you defeat %i%s %s with it. ",
							theItem->charges,
							(theItem->charges == WEAPON_KILLS_TO_AUTO_ID ? "" : " more"),
							(theItem->charges == 1 ? "enemy" : "enemies"));
					strcat(buf, buf2);
				} else if (theItem->category & ARMOR) {
					sprintf(buf2, "It will reveal its secrets to you if you wear it for %i%s turn%s. ",
							theItem->charges,
							(theItem->charges == ARMOR_DELAY_TO_AUTO_ID ? "" : " more"),
							(theItem->charges == 1 ? "" : "s"));
					strcat(buf, buf2);
				} else if (theItem->category & SHIELD) {
					sprintf(buf2, "It will reveal its secrets to you if you wear it for %i%s turn%s. ",
							theItem->charges,
							(theItem->charges == SHIELD_DELAY_TO_AUTO_ID ? "" : " more"),
							(theItem->charges == 1 ? "" : "s"));
					strcat(buf, buf2);
				}
			}			
			
			// Display the known percentage by which the armor/weapon will increase/decrease accuracy/damage/defense if not already equipped.
			if (!(theItem->flags & ITEM_EQUIPPED)) {
				if (theItem->category & WEAPON) {
					current = player.info.accuracy;
					if (rogue.weapon) {
                        currentDamage = (((float) rogue.weapon->damage.lowerBound) + ((float) rogue.weapon->damage.upperBound)) / 2;
						if ((rogue.weapon->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
							current *= pow(WEAPON_ENCHANT_ACCURACY_FACTOR, netEnchant(rogue.weapon));
							currentDamage *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, netEnchant(rogue.weapon));
						} else {
							current *= pow(WEAPON_ENCHANT_ACCURACY_FACTOR, strengthModifier(rogue.weapon));
							currentDamage *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, strengthModifier(rogue.weapon));
						}
					} else {
                        currentDamage = (((float) player.info.damage.lowerBound) + ((float) player.info.damage.upperBound)) / 2;
                    }
					
					new = player.info.accuracy;
					newDamage = (((float) theItem->damage.lowerBound) + ((float) theItem->damage.upperBound)) / 2;
					if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
						new *= pow(WEAPON_ENCHANT_ACCURACY_FACTOR, netEnchant(theItem));
						newDamage *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, netEnchant(theItem));
					} else {
						new *= pow(WEAPON_ENCHANT_ACCURACY_FACTOR, strengthModifier(theItem));
						newDamage *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, strengthModifier(theItem));
					}
					accuracyChange	= (new * 100 / current) - 100;
					damageChange	= (newDamage * 100 / currentDamage) - 100;
					sprintf(buf2, "Wielding the %s%s will %s your current accuracy by %s%i%%%s, and will %s your current damage by %s%i%%%s. ",
							theName,
							((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) ? "" : ", assuming it has no hidden properties,",
							(((short) accuracyChange) < 0) ? "decrease" : "increase",
                            (((short) accuracyChange) < 0) ? badColorEscape : (accuracyChange > 0 ? goodColorEscape : ""),
							abs((short) accuracyChange),
                            whiteColorEscape,
							(((short) damageChange) < 0) ? "decrease" : "increase",
                            (((short) damageChange) < 0) ? badColorEscape : (damageChange > 0 ? goodColorEscape : ""),
							abs((short) damageChange),
                            whiteColorEscape);
					strcat(buf, buf2);
				} else if (theItem->category & ARMOR) {
					new = theItem->armor;
					if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
						new += 10 * netEnchant(theItem);
					} else {
						new += 10 * strengthModifier(theItem);
					}
					new = max(0, new);
					new /= 10;
					sprintf(buf2, "Wearing the %s%s will result in an armor rating of %s%i%s. ",
							theName,
							((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) ? "" : ", assuming it has no hidden properties,",
                            (new > displayedArmorValue() ? goodColorEscape : (new < displayedArmorValue() ? badColorEscape : whiteColorEscape)),
							(int) (new),
                            whiteColorEscape);
					strcat(buf, buf2);
				}
			}
			
			// weapon backstab bonus
			if (theItem->category & WEAPON) {
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					new = netEnchant(theItem);
				} else {
					new = strengthModifier(theItem);
				}
				if (theItem->flags & ITEM_BACKSTAB_X2) {
					new = max(1, 3 + new * 2);
				} else if (theItem->flags & ITEM_BACKSTAB_X1) {
					new = max(1, 3 + new);
				} else if (theItem->flags & ITEM_BACKSTAB_XHALF) {
					new = max(1, 3 + new / 2);
				} else if (theItem->flags & ITEM_BACKSTAB_XQUARTER) {
					new = max(1, 3 + new / 4);
				} else {
					new = 3;
				}
				
				if (!rogue.offhandWeapon || theItem != rogue.weapon) {
					sprintf(buf2, "It does %sx%.2f damage when you sneak attack%s",
							theItem->flags & ITEM_IDENTIFIED ? "" : "about ",
							new,
							theItem->flags & ITEM_RUNIC_IDENTIFIED && theItem->enchant2 != W_SLAYING ? " (which also doubles the chance of the runic activating)" : ""
							);
					strcat(buf, buf2);
					
					// Don't explain this if a runic to save space...
					if (!(theItem->flags & ITEM_RUNIC_IDENTIFIED) && (theItem == rogue.offhandWeapon
																	  || (rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER
																		  && rogue.weapon
																		  && !(theItem->flags & ITEM_EQUIPPED)))) {
						
						if (!(rogue.weapon->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))) {
							sprintf(buf2, "; otherwise this weapon%s make%s every second attack and do%s %sx%.2f damage%s%s. ",
									theItem == rogue.offhandWeapon ? "" : " would",
									theItem == rogue.offhandWeapon ? "s" : "",
									theItem == rogue.offhandWeapon ? "es" : "",
									theItem->flags & ITEM_IDENTIFIED ? "" : "about ",
									rogue.weapon->flags & ITEM_ATTACKS_SLOWLY ? new * 3 / 2 : new / 2,
									theItem == rogue.offhandWeapon ? "" : " if you equipped it as your second weapon",
									rogue.weapon->flags & ITEM_ATTACKS_SLOWLY ? "":", tripled if you equip a mace or warhammer as your first weapon"
									);
							strcat(buf, buf2);						
						} else if ((rogue.weapon->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))
								   == (theItem->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))) {
							sprintf(buf2, "; otherwise this weapon%s make%s every second attack%s. ",
									theItem == rogue.offhandWeapon ? "" : " would",
									theItem == rogue.offhandWeapon ? "s" : "",
									theItem == rogue.offhandWeapon ? "" : " if you equipped it as your second weapon"
									);
							strcat(buf, buf2);
						} else if (theItem->flags & ITEM_ATTACKS_SLOWLY) {
							sprintf(buf2, "; otherwise this weapon %s a chance to hit anything your first weapon misses. ",
									theItem == rogue.offhandWeapon ? "would have" : "has"
									);
							strcat(buf, buf2);						
						} else {
							sprintf(buf2, "; otherwise this weapon %s a chance to hit anything your first weapon misses and do%s %sx%.2f damage%s%s. ",
									theItem == rogue.offhandWeapon ? "would have" : "has",
									theItem == rogue.offhandWeapon ? "es" : "",
									theItem->flags & ITEM_IDENTIFIED ? "" : "about ",
									rogue.weapon->flags & ITEM_ATTACKS_SLOWLY ? new * 3 / 2 : new / 2,
									theItem == rogue.offhandWeapon ? "" : " if you equipped it as your second weapon",
									(rogue.weapon->flags & ITEM_ATTACKS_SLOWLY) && !(theItem->flags & ITEM_ATTACKS_SLOWLY) ? "":", tripled if you equip a mace or warhammer as your first weapon"
									);
							strcat(buf, buf2);						
						}
					} else {
						strcat(buf, ". ");
					}
				}
			
				// Try to explain talisman of sinister a bit better. No space to do this if its a runic though...
				if (rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)) {
					if (theItem == rogue.weapon || !(theItem->flags & ITEM_EQUIPPED)) {
						
						if (rogue.offhandWeapon && (!(theItem->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))
													|| ((theItem->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))
														== (rogue.offhandWeapon->flags & (ITEM_ATTACKS_QUICKLY | ITEM_ATTACKS_SLOWLY | ITEM_ATTACKS_PENETRATE | ITEM_ATTACKS_ALL_ADJACENT))))) {
							
							itemName(rogue.offhandWeapon, theName2, false, false, NULL);
							sprintf(buf2, "\n\nWith your %s as your second weapon, you %sattack twice as quickly%s. ",
									theName2, theItem == rogue.weapon ? "" : " would"
									, theItem == rogue.weapon ? "" : " if this was your first");
							strcat(buf, buf2);
						} else if (theItem->flags & ITEM_ATTACKS_SLOWLY) {
							sprintf(buf2, "\n\nEquipping a mace or hammer as your second weapon will let you attack twice as quickly%s. ",
									theItem == rogue.weapon ? "" : " if you equip this as your first");
							strcat(buf, buf2);
						} else if (theItem->flags & ITEM_ATTACKS_QUICKLY) {
							sprintf(buf2, "\n\nEquipping a rapier as your second weapon will let you attack twice as quickly%s. ",
									theItem == rogue.weapon ? "" : " if you equip this as your first");
							strcat(buf, buf2);
						} else if (theItem->flags & ITEM_ATTACKS_ALL_ADJACENT) {
							sprintf(buf2, "\n\nEquipping an axe or war axe as your second weapon will let you attack twice as quickly%s. ",
									theItem == rogue.weapon ? "" : " if you equip this as your first");
							strcat(buf, buf2);
						} else if (theItem->flags & ITEM_ATTACKS_PENETRATE) {
							sprintf(buf2, "\n\nEquipping a spear or war pike as your second weapon will let you attack twice as quickly%s. ",
									theItem == rogue.weapon ? "" : " if you equip this as your first");
							strcat(buf, buf2);
						} else {
							sprintf(buf2, "\n\nEquipping any weapon as your second weapon will let you attack twice as quickly%s. ",
									theItem == rogue.weapon ? "" : " if you equip this as your first");
							strcat(buf, buf2);
						}
					}				
				}				
			}

			// shield
			if (theItem->category & SHIELD) {
				new = theItem->shieldChance;
				if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
					new += 4 * netEnchant(theItem);
				} else {
					new += 4 * strengthModifier(theItem);
				}
				blockRange = (100 - new) / 3;
				new = clamp(new, 0, 95);
				sprintf(buf2, "%s %s%s will protect you from %i%% of attacks (increasing %sby %i%% per intervening grid so that attacks from more than %i grids away are always blocked). ",
						theItem->flags & ITEM_EQUIPPED ? "Wearing the" : "Your",
						theName,
						((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) ? "" : ", assuming it has no hidden properties,",
						(int) new,
						new > 87 && new < 95 ? "at most " : "", // intermediate values that end up clamped down to 1% per grid increments as they get close to 100%
						new < 95 ? 3 : 1,
						(int) max(5, blockRange)
						);
				strcat(buf, buf2);
				
				sprintf(buf2, "Any blow it stops which would otherwise have cost at least %i%% of your maximum health will weaken the shield, breaking it in %i such blows (%i blows of %i%% of you maximum health if enchanted). ",
						100 * theItem->shieldMinBlow / player.info.maxHP,
						theItem->shieldBlows,
						theItem->shieldBlows + 1,
						100 * (theItem->shieldMinBlow + 1) / player.info.maxHP
						);
				strcat(buf, buf2);

				sprintf(buf2, "%s you will need %i more strength to use weapons and armor and take twice as long to use staffs and wands. ",
					theItem->flags & ITEM_EQUIPPED ? "While this shield is equipped" : "If you equip this shield",					   
					   theItem->kind * 2 + 1
					   );
				strcat(buf, buf2);
				
			}
			
			// protected?
			if (theItem->flags & ITEM_PROTECTED) {
				sprintf(buf2, "%sThe %s cannot be corroded by acid.%s",
                        goodColorEscape,
						theName,
                        whiteColorEscape);
				strcat(buf, buf2);
 			} else if (theItem->flags & ITEM_KNOWN_NOT_CURSED) {
				sprintf(buf2, "The %s %s not cursed.",
						theName,
						theItem->quantity > 1 ? "are" : "is");
				strcat(buf, buf2);
			}
			
			// Fall through
			
		case KEY:
			
			if ((theItem->category & WEAPON) || ((theItem->category & KEY) && (theItem->kind == KEY_RUNE_WEAPON))) {
				
				// runic?
				if (itemRunic(theItem) || (theItem->category & KEY)) {
					if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
						sprintf(buf2, "\n\nGlowing runes of %s adorn the %s. ",
								weaponRunicNames[theItem->enchant2],
								(theItem->category & WEAPON ? theName : "meteroic iron"));
						strcat(buf, buf2);
						
						// W_SPEED, W_QUIETUS, W_PARALYSIS, W_MULTIPLICITY, W_SLOWING, W_CONFUSION, W_SLAYING, W_MERCY, W_PLENTY
						
						enchant = netEnchant(theItem);
						if (itemRune(theItem) == W_SLAYING) {
							sprintf(buf2, "%s will never fail to slay a %s in a single %s. ",
									(theItem->category & WEAPON ? "It": "A weapon marked with it"),
									monsterCatalog[theItem->vorpalEnemy].monsterName,
									theItem->kind >= DART ? "throw" : "stroke");
							strcat(buf, buf2);
						} else if (itemRune(theItem) == W_MULTIPLICITY) {
							if ((theItem->category & WEAPON) && ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)) {
								sprintf(buf2, "%i%% of the time that it hits an enemy%s, %i spectral %s%s will spring into being with accuracy and attack power equal to your own, and will dissipate %i turns later. (If the %s is enchanted, %i image%s will appear %i%% of the time, and will last %i turns.)",
										runicWeaponChance(theItem, false, 0),
										(theItem->kind >= DART ? " (or always if it hits when thrown)" : ""),
										weaponImageCount(enchant),
										theName,
										(weaponImageCount(enchant) > 1 ? "s" : ""),
										weaponImageDuration(enchant),
										theName,
										weaponImageCount((float) (enchant + enchantIncrement(theItem))),
										(weaponImageCount((float) (enchant + enchantIncrement(theItem))) > 1 ? "s" : ""),
										runicWeaponChance(theItem, true, (float) (enchant + enchantIncrement(theItem))),
										weaponImageDuration((float) (enchant + enchantIncrement(theItem))));
							} else {
								sprintf(buf2, "Sometimes, when %s hits an enemy%s, spectral %ss will spring into being with accuracy and attack power equal to your own, and will dissipate shortly thereafter.",
										(theItem->category & WEAPON ? "it": "a weapon marked with it"),
										theItem->kind >= DART ? " (or always if it hits when thrown)" : "",
										theName);
							}
							strcat(buf, buf2);
						} else {
							if ((theItem->category & WEAPON) && ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)) {
                                if (runicWeaponChance(theItem, false, 0) < 2
                                    && rogue.strength - player.weaknessAmount + rogue.mightBonus
									- (rogue.shield && !(theItem->category & (SHIELD)) ? rogue.shield->kind * 2 + 1: 0)
									- (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0)
									< theItem->strengthRequired) {
                                    
                                    strcpy(buf2, "Its runic effect will almost never activate because of your inadequate strength, but sometimes, when");
                                } else {
                                    sprintf(buf2, "%i%% of the time that",
                                            runicWeaponChance(theItem, false, 0));
                                }
								strcat(buf, buf2);
							} else {
								strcat(buf, "Sometimes, when");
							}
							sprintf(buf2, " %s hits an enemy%s, %s",
									(theItem->category & WEAPON ? "it": "a weapon marked with it"),
									(theItem->kind >= DART ? " (or always if it hits when thrown)" : ""),
									weaponRunicEffectDescriptions[itemRune(theItem)]);
							strcat(buf, buf2);
							
							if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
								switch (itemRune(theItem)) {
									case W_SPEED:
										strcat(buf, ". ");
										break;
									case W_PARALYSIS:
										sprintf(buf2, " for %i turns. ",
												(int) (weaponParalysisDuration(enchant)));
										strcat(buf, buf2);
										nextLevelState = (int) (weaponParalysisDuration((float) (enchant + enchantIncrement(theItem))));
										break;
									case W_SLOWING:
										sprintf(buf2, " for %i turns. ",
												weaponSlowDuration(enchant));
										strcat(buf, buf2);
										nextLevelState = weaponSlowDuration((float) (enchant + enchantIncrement(theItem)));
										break;
									case W_CONFUSION:
										sprintf(buf2, " for %i turns. ",
												weaponConfusionDuration(enchant));
										strcat(buf, buf2);
										nextLevelState = weaponConfusionDuration((float) (enchant + enchantIncrement(theItem)));
										break;
									case W_MERCY:
										strcpy(buf2, " by 50% of its maximum health. ");
										strcat(buf, buf2);
										break;
									default:
										strcpy(buf2, ". ");
										strcat(buf, buf2);
										break;
								}
								
								if ((theItem->category & WEAPON) && (((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)
									&& runicWeaponChance(theItem, false, 0) < runicWeaponChance(theItem, true, (float) (enchant + enchantIncrement(theItem))))) {
									sprintf(buf2, "(If the %s is enchanted, the chance will increase to %i%%",
											theName,
											runicWeaponChance(theItem, true, (float) (enchant + enchantIncrement(theItem))));
									strcat(buf, buf2);
									if (nextLevelState) {
										sprintf(buf2, " and the duration will increase to %i turns.)",
												nextLevelState);
									} else {
										strcpy(buf2, ".)");
									}
									strcat(buf, buf2);
								}
							} else {
								strcat(buf, ". ");
							}
						}
						
					} else if (theItem->flags & ITEM_IDENTIFIED) {
						sprintf(buf2, "\n\nGlowing runes of an indecipherable language run down the length of the %s. ",
								(theItem->category & WEAPON ? theName : "meteroic iron"));
						strcat(buf, buf2);
					}
				}
				
				// equipped? cursed?
				if (theItem->flags & ITEM_EQUIPPED) {
					sprintf(buf2, "\n\nYou hold the %s at the ready%s. ",
							theName,
							((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to let go" : ""));
					strcat(buf, buf2);
				} else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
						   && (theItem->flags & ITEM_CURSED)) {
					sprintf(buf2, "\n\nYou can feel a malevolent magic lurking within the %s. ", theName);
					strcat(buf, buf2);
				}
				
			} else if ((theItem->category & (ARMOR | SHIELD)) || ((theItem->category & KEY) && (theItem->kind == KEY_RUNE_ARMOR))) {
				
				// runic?
				if (itemRunic(theItem) || (theItem->category & KEY)) {
					if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
						sprintf(buf2, "\n\nGlowing runes of %s adorn the %s. ",
								armorRunicNames[itemRune(theItem)],
								((theItem->category & ARMOR) || (theItem->category & SHIELD) ? theName : "diamond"));
						strcat(buf, buf2);
						
						// A_MULTIPLICITY, A_MUTUALITY, A_ABSORPTION, A_REPRISAL, A_IMMUNITY, A_REFLECTION, A_BURDEN, A_VULNERABILITY, A_IMMOLATION
						enchant = netEnchant(theItem);
						switch (itemRune(theItem)) {
							case A_MULTIPLICITY:
								sprintf(buf3, "%i", armorImageCount(enchant));
								sprintf(buf2, "When %s, 33%% of the time that an enemy's attack connects, %s allied spectral duplicate%s of your attacker will appear for 3 turns. ",
										(theItem->category & (ARMOR | SHIELD) ? "worn" : "stamped on an armor or shield"),
										(theItem->category & (ARMOR | SHIELD) ? buf3 : "several"),
										(armorImageCount(enchant) == 1 || (theItem->category & (KEY)) ? "" : "s"));
								if (armorImageCount((float) enchant + enchantIncrement(theItem)) > armorImageCount(enchant)) {
									sprintf(buf3, "(If the %s is enchanted, the number of duplicates will increase to %i.) ",
											theName,
											(armorImageCount((float) enchant + enchantIncrement(theItem))));
									strcat(buf2, buf3);
								}
								break;
							case A_MUTUALITY:
								sprintf(buf2, "When %s, the damage that you incur from physical attacks will be split evenly among yourself and all other adjacent enemies. ",
										(theItem->category & (ARMOR | SHIELD) ? "worn" : "stamped on an armor or shield"));
								break;
							case A_ABSORPTION:
								sprintf(buf2, "%s will reduce the damage of inbound attacks by a random amount between 0 and %i, which is %i%% of your current maximum health. (If the %s is enchanted, this maximum amount will %s %i.) ",
										(theItem->category & (ARMOR | SHIELD) ? "It" : "Any armor or shield stamped with it"),
										(int) armorAbsorptionMax(enchant),
										(int) (100 * armorAbsorptionMax(enchant) / player.info.maxHP),
										theName,
										(armorAbsorptionMax(enchant) == armorAbsorptionMax((float) (enchant + enchantIncrement(theItem))) ? "remain at" : "increase to"),
										(int) armorAbsorptionMax((float) (enchant + enchantIncrement(theItem))));
								break;
							case A_REPRISAL:
								if (theItem->category & (ARMOR | SHIELD)) {
									sprintf(buf2, "Any enemy that attacks you will itself be wounded by %i%% of the damage that it inflicts. (If the %s is enchanted, this percentage will increase to %i%%.) ",
										armorReprisalPercent(enchant),
										theName,
										armorReprisalPercent((float) (enchant + enchantIncrement(theItem))));
								} else {
									strcpy(buf2, "Any armor or shield stamped with it will wound enemies by a percentage of the damage they inflict. ");
								}
								break;
							case A_IMMUNITY:
								sprintf(buf2, "It offers complete protection from any attacking %s. ",
										monsterCatalog[theItem->vorpalEnemy].monsterName);
								break;
							case A_REFLECTION:
								if ((theItem->category & (ARMOR | SHIELD)) && theItem->enchant1 > 0) {
									short reflectChance = reflectionChance(enchant);
									short reflectChance2 = reflectionChance(enchant + enchantIncrement(theItem));
									sprintf(buf2, "When worn, you will deflect %i%% of incoming spells -- including directly back at their source %i%% of the time. (If the armor is enchanted, these will increase to %i%% and %i%%.)",
											reflectChance,
											reflectChance * reflectChance / 100,
											reflectChance2,
											reflectChance2 * reflectChance2 / 100);
								} else if (theItem->category & (ARMOR | SHIELD)) {
									short reflectChance = reflectionChance(enchant);
									short reflectChance2 = reflectionChance(enchant + enchantIncrement(theItem));
									sprintf(buf2, "When worn, %i%% of your own spells will deflect from their target -- including directly back at you %i%% of the time. (If the armor is enchanted, these will decrease to %i%% and %i%%.)",
											reflectChance,
											reflectChance * reflectChance / 100,
											reflectChance2,
											reflectChance2 * reflectChance2 / 100);
								} else {
									strcpy(buf2, "When stamped on an armor or shield, it will deflect some incoming spells -- including some directly back at their source. ");
								}
								break;
                            case A_DAMPENING:
                                sprintf(buf2, "When %s, it will harmlessly absorb the concussive impact of any explosions (though you may still be burned). ",
										(theItem->category & (ARMOR | SHIELD) ? "worn" : "stamped on an armor or shield"));
                                break;								
							case A_BURDEN:
								strcpy(buf2, "10% of the time it absorbs a blow, it will permanently become heavier -- increasing both the strength requirement and partially increasing protection. ");
								break;
							case A_VULNERABILITY:
								sprintf(buf2, "While it is %s, inbound attacks will inflict twice as much damage. ",
									(theItem->category & (ARMOR | SHIELD) ? "worn" : "stamped on an armor or shield"));
								break;
                            case A_IMMOLATION:
								if (theItem->category & (ARMOR | SHIELD)) {
									sprintf(buf2, "%i of the time it absorbs a blow, it will explode in flames. (If the %s is enchanted, this will %s to %i%% of the time). ",
											armorImmolationChance(enchant), (theItem->category & (ARMOR) ? "armor" : "shield"), (enchant < -0.5 ? "decrease" : "increase"), 
											(int) (enchant + enchantIncrement(theItem)));
								} else {
									strcpy(buf2, "Some of the time it absorbs a blow, the armor or shield it is stamped onto will explode in flames. ");
								}											
								break;
							default:
								break;
						}
						strcat(buf, buf2);
					} else if (theItem->flags & ITEM_IDENTIFIED) {
						sprintf(buf2, "\n\nGlowing runes of an indecipherable language spiral around the %s. ",
								((theItem->category & ARMOR) || (theItem->category & SHIELD) ? theName : "diamond"));
						strcat(buf, buf2);
					}
				}
				
				// equipped? cursed?
				if (theItem->flags & ITEM_EQUIPPED) {
					sprintf(buf2, "\n\nYou are wearing the %s%s. ",
							theName,
							((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to remove it" : ""));
					strcat(buf, buf2);
				} else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
						   && (theItem->flags & ITEM_CURSED)) {
					sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
                            badColorEscape,
                            theName,
                            whiteColorEscape);
					strcat(buf, buf2);
				}
				
			} else if ((theItem->category & KEY) && ((theItem->kind == KEY_RUNE_WAND) || (theItem->kind == KEY_RUNE_STAFF))) {
				// runic?
				if (theItem->flags & ITEM_RUNIC) {
					if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
						strcat(buf, "\n\n");
						sprintf(buf2, "Glowing runes adorn the %s. ",
								(theItem->kind == KEY_RUNE_WAND ? "unusual alloy" : "petrified wood"));
						upperCase(buf2);
						strcat(buf, buf2);
						
						strcat(buf, boltRunicCatalog[theItem->enchant2].description);
						resolveItemEscapes(buf, theItem, false, true);
						switch (theItem->enchant2) {
							case B_PRECISION:
								if (staffTable[BOLT_MIRRORED_TOTEM].identified || wandTable[BOLT_MIRRORED_TOTEM].identified) {
									strcat(buf, "It makes the bolt from a $WANDORSTAFFs of mirrored totems only place one glyph on the path of the bolt next to the mirrored totem. ");
								}
								break;
							case B_DISTANCE:
								if (staffTable[BOLT_BLINKING].identified || wandTable[BOLT_BLINKING].identified) {
									strcat(buf, "It fires a bolt of force from a $WANDORSTAFF of blinking ahead of you, pushing backwards the first monster it hits. ");
								}
								if (staffTable[BOLT_MIRRORED_TOTEM].identified || wandTable[BOLT_MIRRORED_TOTEM].identified) {
									strcat(buf, "It reduces the range further for the bolt from a $WANDORSTAFF of mirrored totems, but covers the floor around you with glyphs when you use it. ");
								}
								break;
							case B_PENETRATING:
								if (staffTable[BOLT_BLINKING].identified || wandTable[BOLT_BLINKING].identified) {
									strcat(buf, "It lets you lunge attack with any weapon against monsters in the path of the bolt from a $WANDORSTAFF of blinking, continuing on to the next monster if you kill the first. ");
								}
								if (staffTable[BOLT_FIRE].identified || wandTable[BOLT_FIRE].identified) {
									strcat(buf, "It makes bolts from a $WANDORSTAFF of fire continue through all monsters it hits. ");
								}
								if (staffTable[BOLT_LIGHTNING].identified || wandTable[BOLT_LIGHTNING].identified) {
									strcat(buf, "It makes bolts from a $WANDORSTAFF of lightning not reflect off walls or monsters. ");
								}
								if (staffTable[BOLT_TUNNELING].identified || wandTable[BOLT_TUNNELING].identified) {
									strcat(buf, "It makes bolts from a $WANDORSTAFF of tunneling have a chance of shattering any reflective monster which does not reflect the bolt. ");
								}
								break;
							case B_EXPLODES:
								if (staffTable[BOLT_BLINKING].identified || wandTable[BOLT_BLINKING].identified) {
									strcat(buf, "It darkens the area around where the bolt from a $WANDORSTAFF of blinking impacts. ");
								}
								break;
						}
						
						if (boltRunicCatalog[theItem->enchant2].flags & (BOLT_PREVENT_ALL_BUT_ELEMENTAL)) {
							strcat(buf, "It can only be applied to a $WANDORSTAFF of fire, lightning, force, poison, tunnelling or blinking. ");
						} else if (boltRunicCatalog[theItem->enchant2].flags & (BOLT_PREVENT_ANY)) {
							n = 0;
							singular = true;
							for (i = 0; i < NUMBER_BOLT_KINDS; i++) {
								if (theItem->kind == KEY_RUNE_STAFF && staffTable[i].identified && !boltRunicAllowedOnItem(theItem->enchant2, i, NULL, false)) {
									n++;
								} else if (theItem->kind == KEY_RUNE_WAND && wandTable[i].identified && !boltRunicAllowedOnItem(theItem->enchant2, i, NULL, false)) {
									n++;
								}
							}
							if (n) {
								strcat(buf, "It has no effect when applied to a $WANDORSTAFF of ");
							}
							for (i = 0; i < NUMBER_BOLT_KINDS; i++) {
								if (theItem->kind == KEY_RUNE_STAFF && staffTable[i].identified && !boltRunicAllowedOnItem(theItem->enchant2, i, NULL, false)) {
									n--;
									if (n > 1 && !singular) {
										strcat(buf, ", ");
									} else if (n == 1 && !singular) {
										strcat(buf, " or ");
									}
									singular = false;
									if (staffTable[i].name[0]) {
										strcat(buf, staffTable[i].name);
									} else {
										strcat(buf, boltTable[i].name);
									}											   
								} else if (theItem->kind == KEY_RUNE_WAND && wandTable[i].identified && !boltRunicAllowedOnItem(theItem->enchant2, i, NULL, false)) {
									n--;
									if (n > 1 && !singular) {
										strcat(buf, ", ");
									} else if (n == 1 && !singular) {
										strcat(buf, " or ");
									}
									singular = false;
									if (wandTable[i].name[0]) {
										strcat(buf, wandTable[i].name);
									} else {
										strcat(buf, boltTable[i].name);
									}											   
								}
							}
							if (n) {
								strcat(buf, ". ");
							}
						}
						
						switch (theItem->enchant2) {
							case B_QUICK:
								strcpy(buf3, " or intense");
								break;
							case B_SLOW:
								strcpy(buf3, " or quick");
								break;
							case B_SELECTIVE:
							case B_PRECISION:
								strcpy(buf3, " or reflecting");
								break;
							case B_BOUNCES:
								strcpy(buf3, ", selective or precise");
								break;
							default:
								strcpy(buf3, "");
								break;
						}
						sprintf(buf2, "You can%s apply this rune to a $WANDORSTAFF which already has %s%s runes. ",
								theItem->enchant2 == B_II ? "" : "not",
								boltRunicCatalog[theItem->enchant2].name, buf3);
						strcat(buf, buf2);
						resolveItemEscapes(buf, theItem, false, false);
					}
				}
			}
			break;			
		case STAFF:
			// collect bolt runic information
			boltHasFlags = 0;
			numberOfBolts = 1; // not used?
			boltPower = 1.0;
			if (theItem->flags & ITEM_RUNIC_IDENTIFIED) {
				for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
					boltHasFlags |= boltRunicCatalog[theItem->boltEnchants[i]].flags;
					numberOfBolts += boltRunicCatalog[theItem->boltEnchants[i]].number;
					boltPower *= boltRunicCatalog[theItem->boltEnchants[i]].power;
				}
			}
			
			enchant = theItem->enchant1;
			
			// charges
			if ((theItem->flags & ITEM_IDENTIFIED)  || rogue.playbackOmniscience) {
				sprintf(buf2, "\n\nThe %s has %i charges remaining out of a maximum of %i charges, and like all staffs, recovers its charges gradually over time. ",
						theName,
						theItem->charges,
						(int)enchant);
				strcat(buf, buf2);
			} else if (theItem->flags & ITEM_MAX_CHARGES_KNOWN) {
				sprintf(buf2, "\n\nThe %s has a maximum of %i charges, and like all staffs, recovers its charges gradually over time. ",
						theName,
						(int)enchant);
				strcat(buf, buf2);
			}
			
			// effect description
			if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) && staffTable[theItem->kind].identified)
				|| rogue.playbackOmniscience) {
				switch (theItem->kind) {
						// STAFF_TELEPORT, STAFF_SLOW, STAFF_NEGATION, STAFF_DOMINATION, STAFF_FORCE,
						// STAFF_LIGHTNING, STAFF_FIRE, STAFF_POISON, STAFF_TUNNELING, STAFF_BLINKING, STAFF_ENTRANCEMENT,
						// STAFF_PLENTY, STAFF_POLYMORPH, STAFF_BECKONING, STAFF_INVISIBILITY, STAFF_HEALING,
						// STAFF_HASTE, STAFF_OBSTRUCTION, STAFF_DISCORD, STAFF_CONJURATION, STAFF_SENTRY, STAFF_PHANTOM, STAFF_ZOMBIE,
						// STAFF_BLOAT, STAFF_NAGA, STAFF_PIXIE, STAFF_SPIDER, STAFF_TOAD
					case STAFF_TELEPORT:
						sprintf(buf2, "This staff teleports away any creature it hits; its range is limited to %i meters. (If the staff is enchanted%s, this will increase to %i meters.)",
								(int)staffTeleportDistance(enchant * boltPower),
								((int)staffTeleportDistance((enchant + 1) * boltPower)) == ((int)staffTeleportDistance(enchant * boltPower)) ? " twice" : "",
								(int)staffTeleportDistance((enchant + (((int)staffTeleportDistance((enchant + 1) * boltPower)) == ((int)staffTeleportDistance(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_SLOW:
						sprintf(buf2, "This staff slows any creature it hits for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								(int)boltSlowDuration(enchant * boltPower),
								((int)boltSlowDuration((enchant + 1) * boltPower)) == ((int)boltSlowDuration(enchant * boltPower)) ? " twice" : "",
								(int)boltSlowDuration((enchant + (((int)boltSlowDuration((enchant + 1) * boltPower)) == ((int)boltSlowDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));								
						break;
					case STAFF_NEGATION:
						sprintf(buf2, "This staff negates the abilities on any creature it hits for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								(int)staffNegationDuration(enchant * boltPower),
								((int)staffNegationDuration((enchant + 1) * boltPower)) == ((int)staffNegationDuration(enchant * boltPower)) ? " twice" : "",
								(int)staffNegationDuration((enchant + (((int)staffNegationDuration((enchant + 1) * boltPower)) == ((int)staffNegationDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_DOMINATION:
						sprintf(buf2, "This staff negates dominates any wandering creature it hits for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								(int)staffDominationDuration(enchant * boltPower),
								((int)staffTeleportDistance((enchant + 1) * boltPower)) == ((int)staffTeleportDistance(enchant * boltPower)) ? " twice" : "",
								(int)staffDominationDuration((enchant + 1) * boltPower));
						break;
					case STAFF_FORCE:
						sprintf(buf2, "The bolt fired by this staff will knock back the first monster it hits %i metres in the direction of line of fire, through doors and foliage. (If the staff is enchanted%s, this will increase to %i metres and%s its average damage if the target hits an obstruction will increase by %i%%.)",
								(int) (enchant * boltPower),
								((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? " twice" : "",
								(int) ((enchant + (((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? 2 : 1)) * boltPower),
								((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? " if enchanted once" : "",
								(int) (100 * (boltDamageLow((enchant + 1) * boltPower) + boltDamageHigh((enchant + 1) * boltPower)) / (boltDamageLow(enchant * boltPower) + boltDamageHigh(enchant * boltPower)) - 100));
						break;						
					case STAFF_LIGHTNING:
						sprintf(buf2, "This staff deals damage to every creature in its line of fire; nothing is immune. (If the staff is enchanted, its average damage will increase by %i%%.)",
								(int) (100 * (boltDamageLow((enchant + 1) * boltPower) + boltDamageHigh((enchant + 1) * boltPower)) / (boltDamageLow(enchant * boltPower) + boltDamageHigh(enchant * boltPower)) - 100));
						break;
					case STAFF_FIRE:
						sprintf(buf2, "This staff deals damage to any creature that it hits, unless the creature is immune to fire; the bolt will continue inflicting left over damage to targets behind the first if the initial target is killed. (If the staff is enchanted, its average damage will increase by %i%%.) It also sets creatures and flammable terrain on fire.",
								(int) (100 * (boltDamageLow((enchant + 1) * boltPower) + boltDamageHigh((enchant + 1) * boltPower)) / (boltDamageLow(enchant * boltPower) + boltDamageHigh(enchant * boltPower)) - 100));
						break;
					case STAFF_POISON:
						sprintf(buf2, "The bolt from this staff will poison any creature that it hits for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltPoison(enchant * boltPower),
								((int)boltPoison((enchant + 1) * boltPower)) == ((int)boltPoison(enchant * boltPower)) ? " twice" : "",
								boltPoison((enchant + (((int)boltPoison((enchant + 1) * boltPower)) == ((int)boltPoison(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_TUNNELING:
						sprintf(buf2, "The bolt from this staff will dissolve %i layers of obstruction. (If the staff is enchanted%s, this will increase to %i layers.)",
								(int) (enchant * boltPower),
								((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? " twice" : "",
								(int) ((enchant + (((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_BLINKING:
						sprintf(buf2, "This staff enables you to teleport up to %i meters. (If the staff is enchanted%s, this will increase to %i meters.) It recharges half as quickly as most other kinds of staffs.",
								boltBlinkDistance(enchant * boltPower),
								((int)boltBlinkDistance((enchant + 1) * boltPower)) == ((int)boltBlinkDistance(enchant * boltPower)) ? " twice" : "",
								boltBlinkDistance((enchant + (((int)boltBlinkDistance((enchant + 1) * boltPower)) == ((int)boltBlinkDistance(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_ENTRANCEMENT:
						sprintf(buf2, "This staff will compel its target to mirror your movements for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltEntrancementDuration(enchant * boltPower),
								((int)boltEntrancementDuration((enchant + 1) * boltPower)) == ((int)boltEntrancementDuration(enchant * boltPower)) ? " twice" : "",
								boltEntrancementDuration((enchant + (((int)boltEntrancementDuration((enchant + 1) * boltPower)) == ((int)boltEntrancementDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_PLENTY:
						sprintf(buf2, "This staff makes a spectral copy of any monster it hits which lasts for %i turns and fights alongside you. (If the staff is enchanted%s, this will increase to %i turns.)",
								staffPlentyDuration(enchant * boltPower),
								((int)staffPlentyDuration((enchant + 1) * boltPower)) == ((int)staffPlentyDuration(enchant * boltPower)) ? " twice" : "",
								staffPlentyDuration((enchant + (((int)staffPlentyDuration((enchant + 1) * boltPower)) == ((int)staffPlentyDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_POLYMORPH:
						sprintf(buf2, "This staff polymorphs the target monster into another for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								staffPolymorphDuration(enchant * boltPower),
								((int)staffPolymorphDuration((enchant + 1) * boltPower)) == ((int)staffPolymorphDuration(enchant * boltPower)) ? " twice" : "",
								staffPolymorphDuration((enchant + (((int)staffPolymorphDuration((enchant + 1) * boltPower)) == ((int)staffPolymorphDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_BECKONING:
						sprintf(buf2, "This staff yanks any creature it hits towards you; its range is limited to %i meters. (If the staff is enchanted%s, this will increase to %i meters.)",
								staffBeckoningDistance(enchant * boltPower),
								((int)staffBeckoningDistance((enchant + 1) * boltPower)) == ((int)staffBeckoningDistance(enchant * boltPower)) ? " twice" : "",
								staffBeckoningDistance((enchant + (((int)staffBeckoningDistance((enchant + 1) * boltPower)) == ((int)staffBeckoningDistance(enchant * boltPower)) ? 2 : 1)) * boltPower));								
						break;
					case STAFF_INVISIBILITY:
						sprintf(buf2, "This staff turns any creature it hits invisible for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltInvisibilityDuration(enchant * boltPower),
								((int)boltInvisibilityDuration((enchant + 1) * boltPower)) == ((int)boltInvisibilityDuration(enchant * boltPower)) ? " twice" : "",
								boltInvisibilityDuration((enchant + (((int)boltInvisibilityDuration((enchant + 1) * boltPower)) == ((int)boltInvisibilityDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));								
						break;
					case STAFF_HEALING:
						if (enchant * 10 * boltPower < 100) {
							sprintf(buf2, "This staff will heal its target by %i%% of its maximum health. (If the staff is enchanted, this will increase to %i%%.)",
									(int)(enchant * 10 * boltPower),
									min(100, (int)((enchant + 1) * 10 * boltPower)));
						} else {
							strcpy(buf2, "This staff will completely heal its target.");	
						}
						break;
					case STAFF_HASTE:
						sprintf(buf2, "This staff will cause its target to move twice as fast for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltHasteDuration(enchant * boltPower),
								((int)boltHasteDuration((enchant + 1) * boltPower)) == ((int)boltHasteDuration(enchant * boltPower)) ? " twice" : "",
								boltHasteDuration((enchant + (((int)boltHasteDuration((enchant + 1) * boltPower)) == ((int)boltHasteDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_OBSTRUCTION:
						strcpy(buf2, "This staff recharges half as quickly as most other kinds of staffs.");
						break;
					case STAFF_DISCORD:
						sprintf(buf2, "This staff will cause discord for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltDiscordDuration(enchant * boltPower),
								((int)boltDiscordDuration((enchant + 1) * boltPower)) == ((int)boltDiscordDuration(enchant * boltPower)) ? " twice" : "",
								boltDiscordDuration((enchant + (((int)boltDiscordDuration((enchant + 1) * boltPower)) == ((int)boltDiscordDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_NATURE:
						sprintf(buf2, "The plants created by this staff must have %i metres space between them. (If the staff is enchanted%s, this will decrease to %i metres.)",
								staffNatureMinimumGap(enchant * boltPower),
								((int)staffNatureMinimumGap((enchant + 1) * boltPower)) == ((int)staffNatureMinimumGap(enchant * boltPower)) ? " twice" : "",
								staffNatureMinimumGap((enchant + (((int)staffNatureMinimumGap((enchant + 1) * boltPower)) == ((int)staffNatureMinimumGap(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_DETONATION:
						sprintf(buf2, "Creatures hit by this bolt have a life span of %i to %i turns. (If the staff is enchanted, this will decrease to between %i and %i turns.)",
								detonationLifeSpanMinimum(enchant * boltPower),
								detonationLifeSpanMaximum(enchant * boltPower),
								detonationLifeSpanMinimum((enchant + 1) * boltPower),
								detonationLifeSpanMaximum((enchant + 1) * boltPower));
						break;
					case STAFF_CONJURATION:
						sprintf(buf2, "%i phantom blades will be called into service. (If the staff is enchanted%s, this will increase to %i blades.)",
								boltBladeCount(enchant * boltPower),
								((int)boltBladeCount((enchant + 1) * boltPower)) == ((int)boltBladeCount(enchant * boltPower)) ? " twice" : "",
								boltBladeCount((enchant + (((int)boltBladeCount((enchant + 1) * boltPower)) == ((int)boltBladeCount(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;
					case STAFF_PROTECTION:
						sprintf(buf2, "This staff will shield a creature for up to 20 turns against up to %i damage. (If the staff is enchanted%s, this will increase to %i damage.)",
								boltProtection(enchant * boltPower) / 10,
								((int)boltProtection((enchant + 1) * boltPower) / 10) == ((int)boltBladeCount(enchant * boltPower) / 10) ? " twice" : "",
								boltProtection((enchant + (((int)boltProtection((enchant + 1) * boltPower) / 10) == ((int)boltBladeCount(enchant * boltPower) / 10) ? 2 : 1)) * boltPower) / 10);
						break;
					case STAFF_REFLECTION:
						sprintf(buf2, "This staff will make a creature reflective against magical bolts for %i turns. (If the staff is enchanted%s, this will increase to %i turns.)",
								boltReflectionDuration(enchant * boltPower),
								((int)boltReflectionDuration((enchant + 1) * boltPower)) == ((int)boltReflectionDuration(enchant * boltPower)) ? " twice" : "",
								boltReflectionDuration((enchant + (((int)boltReflectionDuration((enchant + 1) * boltPower)) == ((int)boltReflectionDuration(enchant * boltPower)) ? 2 : 1)) * boltPower));
						break;						
					case STAFF_SENTRY:
						sprintf(buf2, "The turret will be protected for %i damage and discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this will become protection from %i damage and %i turns of discord.)",
								boltSentryProtection(enchant * boltPower),
								boltSentryDuration(enchant * boltPower),
								boltSentryProtection((enchant + 1) * boltPower),
								boltSentryDuration((enchant + 1) * boltPower));
						break;
						//					case STAFF_PHANTOM:
						//						sprintf(buf2, "The summoned phantom will be discordant for %i turns before becoming hostile towards you and has a %i%% chance of being surrounded by darkness. (If the staff is enchanted this will become %i turns of discord and %i%% chance of darkness.)",
						//								boltPhantomDuration(enchant * boltPower),
						//								boltPhantomDarkness(enchant * boltPower),
						//								boltPhantomDuration((enchant + 1) * boltPower),
						//								boltPhantomDarkness(enchant + 1) * boltPower);
						//						break;
					case STAFF_ZOMBIE:
						sprintf(buf2, "The summoned zombie will be discordant for %i turns before becoming hostile towards you and has a %i%% chance of being set on fire. (If the staff is enchanted this will become %i turns of discord and %i%% chance of being aflame.)",
								boltZombieDuration(enchant * boltPower),
								staffZombieAflame(enchant * boltPower),
								boltZombieDuration((enchant + 1) * boltPower),
								staffZombieAflame((enchant + 1) * boltPower));
						break;
					case STAFF_BLOAT:
						sprintf(buf2, "Bloats summoned by a staff are not discordant.");
						//sprintf(buf2, "The summoned bloat will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this will become %i turns of discord.)",
						//		boltBloatDuration(enchant * boltPower),
						//		boltBloatDuration(enchant * boltPower));
						break;
					case STAFF_NAGA:
						sprintf(buf2, "The summoned naga will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this will become %i turns of discord.)",
								boltNagaDuration(enchant * boltPower),
								boltNagaDuration((enchant + 1) * boltPower));
						break;
					case STAFF_PIXIE:
						sprintf(buf2, "This staff summons a pixie at the terminus of the bolt. The pixie will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this will become %i turns of discord.)",
								boltPixieDuration(enchant * boltPower),
								boltPixieDuration((enchant + 1) * boltPower));
						break;
					case STAFF_SPIDER:
						sprintf(buf2, "The summoned spider will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this will become %i turns of discord.)",
								boltSpiderDuration(enchant * boltPower),
								boltSpiderDuration((enchant + 1) * boltPower));
						break;
					case STAFF_TOAD:
						sprintf(buf2, "The staff summons one to %i toads, which will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted this summons one to %i toads affected by %i turns of discord.)",
								max(2, (int) (enchant)),
								boltToadDuration(enchant * boltPower),
								max(2, (int) (enchant + 1)),
								boltToadDuration((enchant + 1) * boltPower));
						break;
					case STAFF_UNDERWORM:
						sprintf(buf2, "The summoned underworm will be in a burrow up to %i metres deep and will be discordant for %i turns before becoming hostile towards you. (If the staff is enchanted%s this will become %i metres deep and%s %i turns of discord.)",
								(int)(enchant * boltPower),
								boltUnderwormDuration(enchant * boltPower),
								((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? " twice" : "",
								(int) ((enchant + (((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? 2 : 1)) * boltPower),
								((int) ((enchant + 1) * boltPower)) == ((int)(enchant * boltPower)) ? " if enchanted once" : "",
								boltUnderwormDuration((enchant + 1) * boltPower));
						break;
					case STAFF_MANDRAKE:
						sprintf(buf2, "The planted mandrake will stay alive for %i turns once it emerges from its sac (If the staff is enchanted this will become %i turns.)",
								boltMandrakeDuration(enchant * boltPower),
								boltMandrakeDuration((enchant + 1) * boltPower));
						break;
					case STAFF_DEAD_MANS_EAR:
						sprintf(buf2, "The planted fungus will stay alive for %i turns (If the staff is enchanted this will become %i turns.)",
								boltDeadMansEarDuration(enchant * boltPower),
								boltDeadMansEarDuration((enchant + 1) * boltPower));
						break;
					case STAFF_CRIMSON_CAP:
						sprintf(buf2, "The planted fungus will stay alive for %i turns (If the staff is enchanted this will become %i turns.)",
								boltCrimsonCapDuration(enchant * boltPower),
								boltCrimsonCapDuration((enchant + 1) * boltPower));
						break;
					case STAFF_MIRRORED_TOTEM:
						sprintf(buf2, "The totem will stay alive for %i turns (If the staff is enchanted this will become %i turns.)",
								boltMirroredTotemDuration(enchant * boltPower),
								boltMirroredTotemDuration((enchant + 1) * boltPower));
						break;
					default:
						strcpy(buf2, "No one knows what this staff does.");
						break;
				}
				strcat(buf, "\n\n");
				strcat(buf, buf2);
			}
			// display bolt runic information
			if (theItem->flags & ITEM_RUNIC_IDENTIFIED) {
				strcat(buf, "\n\n");
				for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
					if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_DISTANCE)) {
						strcat(buf, "The focused rune fires a bolt of force from $WANDORSTAFF of blinking ahead of you, pushing backwards the first monster it hits. ");
					} else if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune lets you lunge attack with any weapon against monsters in the path of the bolt from $WANDORSTAFF of blinking, continuing on to the next monster if you kill the first. ");
					} else if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_EXPLODES)) {
						strcat(buf, "The exploding rune darkens the area around where the bolt from $WANDORSTAFF of blinking impacts. ");
					} else if (theItem->kind == BOLT_DETONATION && (theItem->boltEnchants[i] & BOLT_EXPLODES)) {
						strcat(buf, "The exploding rune triggers an explosion where the bolt from $WANDORSTAFF of detonation impacts, instead of creating a delayed detonation in the target creature. ");
					} else if (theItem->kind == BOLT_FIRE && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of fire continue through all monsters it hits. ");
					} else if (theItem->kind == BOLT_FIRE && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of lightning not reflect off walls or monsters. ");
					} else if (theItem->kind == BOLT_TUNNELING && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of tunneling have a chance of shattering any reflective monster which does not reflect the bolt. ");
					} else if (theItem->kind == BOLT_MIRRORED_TOTEM && (theItem->boltEnchants[i] & BOLT_PRECISION)) {
						strcat(buf, "The precise rune makes the bolt from $WANDORSTAFF of mirrored totems only place one glyph on the path of the bolt next to the mirrored totem. ");
					} else if (theItem->kind == BOLT_MIRRORED_TOTEM && (theItem->boltEnchants[i] & BOLT_DISTANCE)) {
						strcat(buf, "The focused rune reduces the range of the bolt from $WANDORSTAFF of mirrored totems, but covers the floor around you with glyphs when you use it. ");
					} else {
						strcat(buf, boltRunicCatalog[theItem->boltEnchants[i]].description);
					}
				}
				resolveItemEscapes(buf, theItem, true, false);
			}
			
			break;
			
		case WAND:
			strcat(buf, "\n\n");
			if ((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) || rogue.playbackOmniscience) {
				if (theItem->charges) {
					sprintf(buf2, "%i charge%s remain%s. A scroll of recharging will add 1 charge, and enchanting this wand will add %i charge%s.",
							theItem->charges,
							(theItem->charges == 1 ? "" : "s"),
							(theItem->charges == 1 ? "s" : ""),
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
				} else {
					sprintf(buf2, "No charges remain.  A scroll of recharging will add 1 charge, and enchanting this wand will add %i charge%s.",
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
				}
			} else {
				if (theItem->enchant2) {
					sprintf(buf2, "You have used this wand %i time%s, but do not know how many charges, if any, remain.",
							theItem->enchant2,
							(theItem->enchant2 == 1 ? "" : "s"));
				} else {
					strcpy(buf2, "You have not yet used this wand.");
				}
				
				if (wandTable[theItem->kind].identified) {
					strcat(buf, buf2);
					sprintf(buf2, " Wands of this type can be found with %i to %i charges. Enchanting this wand will add %i charge%s.",
							wandTable[theItem->kind].range.lowerBound,
							wandTable[theItem->kind].range.upperBound,
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
				}
			}
			strcat(buf, buf2);
			// display bolt runic information
			if (theItem->flags & ITEM_RUNIC_IDENTIFIED) {
				strcat(buf, "\n\n");
				for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
					if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_DISTANCE)) {
						strcat(buf, "The focused rune fires a bolt of force from $WANDORSTAFF of blinking ahead of you, pushing the first monster it hits. ");
					} else if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune lets you lunge attack with any weapon against monsters in the path of the bolt from $WANDORSTAFF of blinking, continuing on to the next monster if you kill the first. ");
					} else if (theItem->kind == BOLT_BLINKING && (theItem->boltEnchants[i] & BOLT_EXPLODES)) {
						strcat(buf, "The exploding rune darkens the area around where the bolt from $WANDORSTAFF of blinking impacts. ");
					} else if (theItem->kind == BOLT_DETONATION && (theItem->boltEnchants[i] & BOLT_EXPLODES)) {
						strcat(buf, "The exploding rune triggers an explosion where the bolt from $WANDORSTAFF of detonation impacts, instead of creating a delayed detonation in the target creature. ");
					} else if (theItem->kind == BOLT_FIRE && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of fire continue through all monsters it hits. ");
					} else if (theItem->kind == BOLT_FIRE && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of lightning not reflect off walls or monsters. ");
					} else if (theItem->kind == BOLT_TUNNELING && (theItem->boltEnchants[i] & BOLT_PENETRATING)) {
						strcat(buf, "The penetrating rune makes bolts from $WANDORSTAFF of tunneling have a chance of shattering any reflective monster which does not reflect the bolt. ");
					} else if (theItem->kind == BOLT_MIRRORED_TOTEM && (theItem->boltEnchants[i] & BOLT_PRECISION)) {
						strcat(buf, "The precise rune makes the bolt from $WANDORSTAFF of mirrored totems only place one glyph on the path of the bolt next to the mirrored totem. ");
					} else if (theItem->kind == BOLT_MIRRORED_TOTEM && (theItem->boltEnchants[i] & BOLT_DISTANCE)) {
						strcat(buf, "The focused rune reduces the range of the bolt from $WANDORSTAFF of mirrored totems, but covers the floor around you with glyphs when you use it. ");
					} else {
						strcat(buf, boltRunicCatalog[theItem->boltEnchants[i]].description);
					}
				}
				resolveItemEscapes(buf, theItem, true, false);
			}
			
			break;

		case RING:
			// RING_CLAIRVOYANCE, RING_STEALTH, RING_REGENERATION, RING_TRANSFERENCE, RING_LIGHT, RING_AWARENESS, RING_WISDOM,
			// RING_MIGHT, RING_TELEPATHY
			if (((theItem->flags & ITEM_IDENTIFIED) && ringTable[theItem->kind].identified) || rogue.playbackOmniscience) {
				switch (theItem->kind) {
					case RING_CLAIRVOYANCE:
						if (theItem->enchant1 > 0) {
							sprintf(buf2, "\n\nThis ring provides magical sight with a radius of %i. (If the ring is enchanted, this will increase to %i.)",
									theItem->enchant1 + 1,
									theItem->enchant1 + 2);
						} else if (theItem->enchant1 == 0) {
							sprintf(buf2, "\n\nThis ring has no effect. If the ring is enchanted, it will provide magical sight with a radius of 2.");
						} else {
							sprintf(buf2, "\n\nThis ring magically blinds you to a radius of %i. (If the ring is enchanted, this will decrease to %i.)",
									(theItem->enchant1 * -1) + 1,
									(theItem->enchant1 * -1));
						}
						strcat(buf, buf2);
						break;
					case RING_REGENERATION:
						sprintf(buf2, "\n\nWith this ring equipped, you will regenerate all of your health in %li turns (instead of %li). (If the ring is enchanted, this will decrease to %li turns.)",
								(long) (turnsForFullRegen(theItem->enchant1) / 1000),
								(long) TURNS_FOR_FULL_REGEN,
								(long) (turnsForFullRegen(theItem->enchant1 + 1) / 1000));
						strcat(buf, buf2);
						break;
					case RING_STEALTH:
						if (rogue.armor) {
							noise = rogue.armor->kind;
						}
						else {
							noise = -1;
						}
						if (theItem->enchant1 < 0) {
							noise += theItem->enchant1 * 4;
						}
						else {
							noise -= theItem->enchant1;
						}
						sprintf(buf2, "\n\nWearing this ring makes you%s%s %s %s. (If the ring is enchanted, you%s%s will be %s %s.)",
								rogue.armor ? "r " : "", rogue.armor ? armorTable[rogue.armor->kind].name : "",
								theItem->enchant1 < 0 ? (noise >= NUMBER_ARMOR_KINDS ? "louder than" : "as loud as") : (noise < 0 ? "quieter than" : "as quiet as"),
								armorTable[clamp(noise, 0, NUMBER_ARMOR_KINDS-1)].name,
								rogue.armor ? "r " : "", rogue.armor ? armorTable[rogue.armor->kind].name : "",
								theItem->enchant1 < 0 ? (noise - 4 >= NUMBER_ARMOR_KINDS ? "louder than" : "as loud as") : (noise - 1 < 0 ? "quieter than" : "as quiet as"),
								armorTable[theItem->enchant1 < 0 ? clamp(noise-4, 0, NUMBER_ARMOR_KINDS-1) : clamp(noise-1, 0, NUMBER_ARMOR_KINDS-1)].name);
						strcat (buf, buf2);
						break;
					case RING_TRANSFERENCE:
						if (theItem->enchant1) {
							sprintf(buf2, "\n\nEach blow you land will %s you by %i%% of the damage you inflict. (If the ring is enchanted, this will %s to %i%%.)",
								(theItem->enchant1 > 0 ? "heal" : "harm"),
								abs(theItem->enchant1) * 10,
								(theItem->enchant1 > 0 ? "increase" : "decrease"),
								abs(theItem->enchant1 + 1) * 10);
						} else {
							sprintf(buf2, "\n\nIf this ring is enchanted%s, each blow you land will heal you by 10%% of the damage you inflict.",
									(theItem->flags & ITEM_EQUIPPED ? "" : " and worn"));
						}
						strcat(buf, buf2);
						break;
					case RING_WISDOM:
						if (theItem->enchant1) {
							sprintf(buf2, "\n\nWhen worn, your staffs will recharge at %i%% of their normal rate. (If the ring is enchanted, the rate will increase to %i%% of the normal rate.)",
								(int) (100 * pow(1.3, min(27, theItem->enchant1))),
								(int) (100 * pow(1.3, min(27, (theItem->enchant1 + 1)))));
						} else {
							sprintf(buf2, "\n\nIf this ring is enchanted%s, your staffs will recharge at %i%% of their normal rate.",
									(theItem->flags & ITEM_EQUIPPED ? "" : " and worn"),
									(int) (100 * pow(1.3, min(27, 1))));
						}
						strcat(buf, buf2);
						break;
					case RING_TELEPATHY:
						if (theItem->enchant1 > 0) {
							sprintf(buf2, "\n\nThis ring provides telepathy with a radius of %i. (If the ring is enchanted, this will increase to %i.)",
									theItem->enchant1 + 2,
									theItem->enchant1 + 3);
						} else if (theItem->enchant1 == 0) {
							sprintf(buf2, "\n\nThis ring has no effect. If the ring is enchanted, it will provide magical sight with a radius of 2.");
						} else {
							sprintf(buf2, "\n\nThis ring magically blinds you to monsters out to a radius of %i. (If the ring is enchanted, this will decrease to %i.)",
									(theItem->enchant1 * -1) + 2,
									(theItem->enchant1 * -1) + 1);
						}
						strcat(buf, buf2);
						break;
					default:
						break;
				}
			} else {
				sprintf(buf2, " It will reveal its secrets to you if you wear it for %i%s turn%s.",
						theItem->charges,
						(theItem->charges == RING_DELAY_TO_AUTO_ID ? "" : " more"),
						(theItem->charges == 1 ? "" : "s"));
				strcat(buf, buf2);
			}
			
			// equipped? cursed?
			if (theItem->flags & ITEM_EQUIPPED) {
				sprintf(buf2, "\n\nThe %s is on your finger%s. ",
						theName,
						((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to remove it" : ""));
				strcat(buf, buf2);
			} else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
					   && (theItem->flags & ITEM_CURSED)) {
				sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
						badColorEscape,
						theName,
						whiteColorEscape);
				strcat(buf, buf2);
			}
			break;
			
        case CHARM:
			enchant = theItem->enchant1;
            switch (theItem->kind) {
                case CHARM_HEALTH:
                    sprintf(buf2, "\n\nWhen used, the charm will heal %i%% of your health and recharge in %i turns. (If the charm is enchanted, it will heal %i%% of your health and recharge in %i turns.)",
                            charmHealing(enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmHealing(enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_PROTECTION:
				case CHARM_PROTECTION_2:
                    sprintf(buf2, "\n\nWhen used, the charm will shield you for up to %i turns against up to %i damage and recharge in %i turns. (If the charm is enchanted, this will change to %i turns for %i damage and recharge in %i turns.)",
                            min(charmEffectDuration(theItem->kind, enchant), charmProtection(enchant)),
							charmProtection(enchant) / 10,
                            charmRechargeDelay(theItem->kind, enchant),
                            min(charmEffectDuration(theItem->kind, enchant + 1), charmProtection(enchant + 1)),
                            charmProtection(enchant + 1) / 10,
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_HASTE:
                    sprintf(buf2, "\n\nWhen used, the charm will haste you for %i turns and recharge in %i turns. (If the charm is enchanted, the haste will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_FIRE_IMMUNITY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you immunity to fire for %i turns and recharge in %i turns. (If the charm is enchanted, the immunity will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_INVISIBILITY:
                    sprintf(buf2, "\n\nWhen used, the charm will turn you invisible for %i turns and recharge in %i turns. While invisible, monsters more than two meters away cannot track you. (If the charm is enchanted, the invisibility will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_TELEPATHY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you telepathy for %i turns and recharge in %i turns. (If the charm is enchanted, the telepathy will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_LEVITATION:
                    sprintf(buf2, "\n\nWhen used, the charm will lift you off the ground for %i turns and recharge in %i turns. (If the charm is enchanted, the levitation will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_SHATTERING:
                    sprintf(buf2, "\n\nWhen used, the charm will dissolve the nearby walls and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_CAUSE_FEAR:
                    sprintf(buf2, "\n\nWhen used, the charm will terrify all visible creatures and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_TELEPORTATION:
                    sprintf(buf2, "\n\nWhen used, the charm will teleport you elsewhere in the dungeon and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_RECHARGING:
                    sprintf(buf2, "\n\nWhen used, the charm will recharge your staffs (though not your wands or charms), after which it will recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
                case CHARM_NEGATION:
                    sprintf(buf2, "\n\nWhen used, the charm will wipe all magical effects off of creatures in your field of view and items on the ground, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
					
				case CHARM_STRENGTH:
                    sprintf(buf2, "\n\nWhen used, the charm will cure you of any weakness and temporarily boost your strength by +%i for %i turns, recharging in %i turns. (If the charm is enchanted, the +%i strength boost will last for %i turns and it will recharge in %i turns.)",
							theItem->enchant1,
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
							theItem->enchant1 + 1,
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
                    break;
				case CHARM_DETECT_MAGIC:
                    sprintf(buf2, "\n\nWhen used, the charm detect all magic items on the level and in your pack, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_WINDS:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release swirling winds which disperse gases and block thrown weapons, darts and arrows, recharging in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_POISONOUS_GAS:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release a cloud of caustic gas, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_PARALYSIS:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release a cloud of paralytic gas, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_DISCORD:
                    sprintf(buf2, "\n\nWhen used, the charm will make all monsters in line of sight discordant for %i turns, and recharge in %i turns. (If the charm is enchanted, the discord will last for %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant),
                            charmEffectDuration(theItem->kind, enchant + 1),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_CONFUSION:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release a cloud of confusing gas, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_INCINERATION:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will explode in flames, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_DARKNESS:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will darken the surrounding area, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_DESCENT:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will cause the floor to disappear, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_CREEPING_DEATH:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release deadly lichen spores, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_WATER:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will flood the area with water, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_STENCH:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release a cloud of putrescence, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_EXPLOSIVE_GAS:
                    sprintf(buf2, "\n\nWhen used or thrown, the charm will release a cloud of explosive gas, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_CHARMING:
                    sprintf(buf2, "\n\nWhen used, the charm will enchant any other charm with less than +%i enchantment, and recharge in %i turns. (If the charm is enchanted, it can affect charms with less than +%i enchantment and will recharge in %i turns.)",
							theItem->enchant1,
                            charmRechargeDelay(theItem->kind, enchant),
							theItem->enchant1 + 1,
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_IDENTIFY:
                    sprintf(buf2, "\n\nWhen used, the charm will identify one item, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_MAGIC_MAPPING:
                    sprintf(buf2, "\n\nWhen used, the charm will reveal a map of your surroundings, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_DUPLICATION:
                    sprintf(buf2, "\n\nWhen used, the charm will create a duplicate of any bad weapon, armor, shield, ring, potion or scroll (exception scrolls of duplication), and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_AGGRAVATE_MONSTERS:
                    sprintf(buf2, "\n\nWhen used, the charm will aggravate all monsters in line of sight, preventing them from fleeing or keeping their distance, and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, enchant),
                            charmRechargeDelay(theItem->kind, enchant + 1));
					break;
				case CHARM_SUMMON_MONSTERS:
                    sprintf(buf2, "\n\nWhen used, the charm will summon %s monster%s which will fight alongside you for %i turns before turning on you, and recharge in %i turns. (If the charm is enchanted, the monsters will remain your allies for %i turns before turning on you and it will recharge in %i turns.)",
							(theItem->enchant1 == 1 ? "a" : (theItem->enchant1 == 2 ? "two" : "three")),
							(theItem->enchant1 == 1 ? "" : "s"),
							charmEffectDuration(theItem->kind, enchant),
							charmRechargeDelay(theItem->kind, enchant),
							charmEffectDuration(theItem->kind, enchant + 1),
							charmRechargeDelay(theItem->kind, enchant + 1));
					break;
					
                default:
                    break;
            }
            strcat(buf, buf2);
            break;			
			
		case TALISMAN:
			// TALISMAN_MADNESS, TALISMAN_BALANCE, TALISMAN_FLAMESPIRIT, TALISMAN_SINISTER, TALISMAN_THIRD_EYE, TALISMAN_ASSASSINS
			
			if (((theItem->flags & ITEM_IDENTIFIED) && talismanTable[theItem->kind].identified) || rogue.playbackOmniscience) {
				switch (theItem->kind) {
					case TALISMAN_MADNESS:
						sprintf(buf2, "\n\nThis talisman increases the duration of hallucination from a potion or monster to %i%% of normal. (If the talisman is enchanted, this will %s to %i%%.)",
								(int) (100 * madnessBoost(theItem->enchant1)),
								theItem->enchant1 >= 0 ? "increase" : "decrease",
								(int) (100 * madnessBoost(theItem->enchant1 + 1)));
						strcat(buf, buf2);
						if (theItem->enchant1 > 0) {
							sprintf(buf2, "\n\nWhile you are hallucinating, this talisman causes any monster you hit to suffer %i turns of discord and increases the enchantment of equipped weapons and armor by +%i. (If the talisman is enchanted, this will increase to %i turns and a +%i increase.)",
									madnessDuration(theItem->enchant1),
									theItem->enchant1,
									madnessDuration(theItem->enchant1 + 1),
									theItem->enchant1 + 1);
						}
						strcat(buf, buf2);
						break;
					case TALISMAN_FLAMESPIRIT:
						sprintf(buf2, "\n\nThis talisman increases the duration of you burning to %i%% of normal. (If the talisman is enchanted, this will %s to %i%%.)",
								(int) (100 * fireDancingBoost(theItem->enchant1)),
								theItem->enchant1 >= 0 ? "increase" : "decrease",
								(int) (100 * fireDancingBoost(theItem->enchant1 + 1)));
						strcat(buf, buf2);
						if (theItem->enchant1 > 0) {
							sprintf(buf2, "\n\nWhile you are burning, this increases the enchantment of equipped rings by +%i. (If the talisman is enchanted, this will increase to +%i.)",
									theItem->enchant1,
									theItem->enchant1 + 1);
						}
						strcat(buf, buf2);
						break;
					case TALISMAN_SPIDER:
						sprintf(buf2, "\n\nThis talisman increases the time you are poisoned for to %i%% of normal and %ss you for %i%% of the damage you inflict while poisoned using your current weapon. (If enchanted, this %s to %i%% duration and %i%% damage.)",
								(int) (100 * poisonBoost(theItem->enchant1)),
								theItem->enchant1 >= 0 ? "heal" : "harm",
								rogue.weapon ? (rogue.weapon->flags & ITEM_ATTACKS_PENETRATE ? abs(theItem->enchant1) * 15 :
												(rogue.weapon->flags & ITEM_ATTACKS_ALL_ADJACENT ?
												 abs(theItem->enchant1) * 10 : abs(theItem->enchant1) * 20)) : abs(theItem->enchant1) * 100,
								theItem->enchant1 >= 0 ? "increases" : "reduces",
								(int) (100 * poisonBoost(theItem->enchant1 + 1)),
								rogue.weapon ? (rogue.weapon->flags & ITEM_ATTACKS_PENETRATE ? abs(theItem->enchant1 + 1) * 15 :
												(rogue.weapon->flags & ITEM_ATTACKS_ALL_ADJACENT ?
												 abs(theItem->enchant1 + 1) * 10 : abs(theItem->enchant1 + 1) * 20)) : abs(theItem->enchant1 + 1) * 100
								);
						strcat(buf, buf2);
						break;
					case TALISMAN_BALANCE:
						sprintf(buf2, "\n\nThis talisman increases the enchantment of any beneficial ring you wear by twice the enchantment level of any cursed ring you wear. (If the talisman is enchanted, it will instead increase the malus of any cursed ring you wear.)");
						strcat(buf, buf2);
						break;
					case TALISMAN_SINISTER:
						sprintf(buf2, "\n\nYou will attack twice as fast, alternating blows, if you use a dagger, sword or broadsword as your first weapon or any two weapons that attack the same way; otherwise you will use your second weapon if you miss with your first or sneak attack a monster and your second weapon will get a damage multiplier. Two daggers, swords or broadswords will also get this multiplier, having a mace or warhammer for your first will triple this multiplier, and use your first weapon for sneak attacks, and a mace or warhammer as your second will never get this multiplier.");
						strcat(buf, buf2);
						break;
					case TALISMAN_ASSASSIN:
						sprintf(buf2, "\n\nThe targets highlighted by this talisman will be lost if you remove it, and you only get an assassination target on new levels you visit; not any existing level.");
						strcat(buf, buf2);
						break;
					default:
						break;
				}
				if (theItem->kind > TALISMAN_BALANCE) {
					strcat(buf, " Enchanting this item will have no effect.");
				}
			} else if (theItem->kind <= TALISMAN_BALANCE) {
				strcat(buf, " It will reveal its secrets to you if you use the item that the talisman modifies the effects of.");
			}
			
			// equipped? cursed?
			if (theItem->flags & ITEM_EQUIPPED) {
				sprintf(buf2, "\n\nYou are wearing the %s%s. ",
						theName,
						((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to remove it" : ""));
				strcat(buf, buf2);
			} else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
					   && (theItem->flags & ITEM_CURSED)) {
				sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
						badColorEscape,
						theName,
						whiteColorEscape);
				strcat(buf, buf2);
			}
			break;
		default:
			break;
	}
}

boolean displayMagicCharForItem(item *theItem) {
	if (!(theItem->flags & ITEM_MAGIC_DETECTED)
		|| (theItem->category & PRENAMED_CATEGORY)) {
		return false;
	}
	if (theItem->category & (STAFF | POTION | SCROLL | WAND)) {
		return !(tableForItemCategory(theItem->category)[theItem->kind].identified);
	}
	return true;
}

char displayInventory(unsigned long categoryMask,
					  unsigned long requiredFlags,
					  unsigned long forbiddenFlags,
					  boolean waitForAcknowledge,
					  boolean includeButtons) {
	item *theItem;
	short i, j, m, maxLength = 0, itemNumber, itemCount, equippedItemCount;
	short extraLineCount = 0;
	item *itemList[DROWS];
	char buf[DCOLS*3];
	char theKey;
	rogueEvent theEvent;
	boolean magicDetected, lumenstone, repeatDisplay;
	short highlightItemLine, itemSpaceRemaining;
	cellDisplayBuffer dbuf[COLS][ROWS];
	cellDisplayBuffer rbuf[COLS][ROWS];
	brogueButton buttons[50] = {{{0}}};
	short actionKey;
	color darkItemColor;
	
	char whiteColorEscapeSequence[20],
	grayColorEscapeSequence[20],
	yellowColorEscapeSequence[20],
	darkYellowColorEscapeSequence[20],
	goodColorEscapeSequence[20],
	badColorEscapeSequence[20];
	char *magicEscapePtr;
	
	assureCosmeticRNG;
	
	clearCursorPath();
	clearDisplayBuffer(dbuf);
	
	whiteColorEscapeSequence[0] = '\0';
	encodeMessageColor(whiteColorEscapeSequence, 0, &white);
	grayColorEscapeSequence[0] = '\0';
	encodeMessageColor(grayColorEscapeSequence, 0, &gray);
	yellowColorEscapeSequence[0] = '\0';
	encodeMessageColor(yellowColorEscapeSequence, 0, &itemColor);
	darkItemColor = itemColor;
	applyColorAverage(&darkItemColor, &black, 50);
	darkYellowColorEscapeSequence[0] = '\0';
	encodeMessageColor(darkYellowColorEscapeSequence, 0, &darkItemColor);
	goodColorEscapeSequence[0] = '\0';
	encodeMessageColor(goodColorEscapeSequence, 0, &goodMessageColor);
	badColorEscapeSequence[0] = '\0';
	encodeMessageColor(badColorEscapeSequence, 0, &badMessageColor);
	
	if (packItems->nextItem == NULL) {
		confirmMessages();
		message("Your pack is empty!", false);
		restoreRNG;
		return 0;
	}
	
	magicDetected = false;
	lumenstone = false;
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		if (displayMagicCharForItem(theItem) && (theItem->flags & ITEM_MAGIC_DETECTED)) {
			magicDetected = true;
		}
		if (theItem->category & GEM) {
			lumenstone = true;
		}
	}
	
	// List the items in the order we want to display them, with equipped items at the top.
	itemNumber = 0;
	equippedItemCount = 0;
	// First, the equipped weapon if any.
	if (rogue.weapon) {
		itemList[itemNumber] = rogue.weapon;
		itemNumber++;
		equippedItemCount++;
	}
	// First, the equipped weapon if any.
	if (rogue.offhandWeapon) {
		itemList[itemNumber] = rogue.offhandWeapon;
		itemNumber++;
		equippedItemCount++;
	}
	// Now, the equipped armor if any.
	if (rogue.armor) {
		itemList[itemNumber] = rogue.armor;
		itemNumber++;
		equippedItemCount++;
	}
	// Now, the equipped shield if any.
	if (rogue.shield) {
		itemList[itemNumber] = rogue.shield;
		itemNumber++;
		equippedItemCount++;
	}
	// Now, the equipped rings, if any.
	if (rogue.ringLeft) {
		itemList[itemNumber] = rogue.ringLeft;
		itemNumber++;
		equippedItemCount++;
	}
	if (rogue.ringRight) {
		itemList[itemNumber] = rogue.ringRight;
		itemNumber++;
		equippedItemCount++;
	}
	if (rogue.ringThird) {
		itemList[itemNumber] = rogue.ringThird;
		itemNumber++;
		equippedItemCount++;
	}
	if (rogue.talisman) {
		itemList[itemNumber] = rogue.talisman;
		itemNumber++;
		equippedItemCount++;
	}
	// Now all of the non-equipped items.
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		if (!(theItem->flags & ITEM_EQUIPPED)) {
			itemList[itemNumber] = theItem;
			itemNumber++;
		}
	}
	
	// Initialize the buttons:
	for (i=0; i < max(MAX_PACK_ITEMS, ROWS); i++) {
		buttons[i].y = mapToWindowY(i + (equippedItemCount && i >= equippedItemCount ? 1 : 0));
		buttons[i].buttonColor = black;
		buttons[i].opacity = INTERFACE_OPACITY;
		buttons[i].flags |= B_DRAW;
	}
	// Now prepare the buttons.
	//for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
	for (i=0; i<itemNumber; i++) {
		theItem = itemList[i];
		// Set button parameters for the item:
		buttons[i].flags |= (B_DRAW | B_GRADIENT | B_ENABLED);
		if (!waitForAcknowledge) {
			buttons[i].flags |= B_KEYPRESS_HIGHLIGHT;
		}
		buttons[i].hotkey[0] = theItem->inventoryLetter;
		buttons[i].hotkey[1] = theItem->inventoryLetter + 'A' - 'a';
		
		if (((theItem->category & categoryMask)
			 || ((theItem->category & WEAPON) && theItem->kind == DART && (categoryMask & COATING_DARTS))
			 || ((theItem->category & SCROLL) && theItem->kind != SCROLL_DUPLICATION && (categoryMask & DUPLICATE_SCROLL))
			 || ((theItem->category & TOME) && theItem->kind != TOME_DUPLICATION && (categoryMask & DUPLICATE_TOME))
			 || ((theItem->category & POTION) && (theItem->kind != POTION_LIFE || lumenstone) && (categoryMask & ENCHANT_POTION))
			 || ((theItem->category & (POTION | SCROLL)) && theItem->quantity >= 2 && (categoryMask & COMBINE_2_POTION_OR_SCROLL))
			 || ((theItem->category & CHARM) && theItem->enchant1 < rogue.charming && (categoryMask & CHARMING_CHARM))
			 || ((theItem->category & TALISMAN) && (theItem->kind <= TALISMAN_MAX_ENCHANT || theItem->kind == TALISMAN_BALANCE) && (categoryMask & ENCHANT_TALISMAN))
//			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_CHAOS && (categoryMask & APPLY_TALISMAN))
			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_ALCHEMY && (categoryMask & APPLY_TALISMAN))			 
			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_SHAPE_CHANGING && (categoryMask & APPLY_TALISMAN))) &&
			(!(~(theItem->flags) & requiredFlags) || ((requiredFlags & ITEM_CURSED) && itemMagicChar(theItem) == BAD_MAGIC_CHAR)
			 || ((requiredFlags & ITEM_CAN_BE_IDENTIFIED) && (theItem->category & (WEAPON | ARMOR | SHIELD))
				 && rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY && !(theItem->flags & ITEM_RUNIC_IDENTIFIED))) &&
			!(theItem->flags & forbiddenFlags)) {
			
			buttons[i].flags |= (B_HOVER_ENABLED);
		}
		
		// Set the text for the button:
		itemName(theItem, buf, true, true, (buttons[i].flags & B_HOVER_ENABLED) ? &white : &gray);
		upperCase(buf);
		
		if ((theItem->flags & ITEM_MAGIC_DETECTED)
			&& !(theItem->category & AMULET)) { // Won't include food, keys, lumenstones or amulet.
			buttons[i].symbol[0] = (itemMagicChar(theItem) ? itemMagicChar(theItem) : '-');
			if (buttons[i].symbol[0] == '-') {
				magicEscapePtr = yellowColorEscapeSequence;
			} else if (buttons[i].symbol[0] == GOOD_MAGIC_CHAR) {
				magicEscapePtr = goodColorEscapeSequence;
			} else {
				magicEscapePtr = badColorEscapeSequence;
			}
			
			// The first '*' is the magic detection symbol, e.g. '-' for non-magical.
			// The second '*' is the item character, e.g. ':' for food.
			sprintf(buttons[i].text, " %c%s %s* %s* %s%s%s%s",
					theItem->inventoryLetter,
					(theItem->flags & ITEM_PROTECTED ? "}" : (!(theItem->category & (WEAPON | ARMOR | SHIELD | RING)) || theItem->flags & ITEM_CURSED ? ")" : ">")),
					magicEscapePtr,
					(buttons[i].flags & B_HOVER_ENABLED) ? yellowColorEscapeSequence : darkYellowColorEscapeSequence,
					(buttons[i].flags & B_HOVER_ENABLED) ? whiteColorEscapeSequence : grayColorEscapeSequence,
					buf,
					grayColorEscapeSequence,
					(theItem->flags & ITEM_EQUIPPED ? ((theItem->category & WEAPON) ? (rogue.offhandWeapon == theItem ? " (in offhand) " : " (in hand) ") : " (being worn) ") : ""));
			buttons[i].symbol[1] = theItem->displayChar;
		} else {
			sprintf(buttons[i].text, " %c%s %s%s* %s%s%s%s", // The '*' is the item character, e.g. ':' for food.
					theItem->inventoryLetter,
					(theItem->flags & ITEM_PROTECTED ? "}" : (theItem->flags & ITEM_KNOWN_NOT_CURSED ? ">" : ")")),
					(magicDetected ? "  " : ""), // For proper spacing when this item is not detected but another is.
					(buttons[i].flags & B_HOVER_ENABLED) ? yellowColorEscapeSequence : darkYellowColorEscapeSequence,
					(buttons[i].flags & B_HOVER_ENABLED) ? whiteColorEscapeSequence : grayColorEscapeSequence,
					buf,
					grayColorEscapeSequence,
					(theItem->flags & ITEM_EQUIPPED ? ((theItem->category & WEAPON) ? (rogue.offhandWeapon == theItem ? " (in offhand) " : " (in hand) ") : " (being worn) ") : ""));
			buttons[i].symbol[0] = theItem->displayChar;
		}
		
		// Keep track of the maximum width needed:
		maxLength = max(maxLength, strLenWithoutEscapes(buttons[i].text));

//		itemList[itemNumber] = theItem;
//		
//		itemNumber++;
	}
	//printf("\nMaxlength: %i", maxLength);
	itemCount = itemNumber;
	if (!itemNumber) {
		confirmMessages();
		message("Nothing of that type!", false);
		restoreRNG;
		return 0;
	}
	if (waitForAcknowledge) {
		// Add the two extra lines as disabled buttons.
		itemSpaceRemaining = MAX_PACK_ITEMS - numberOfItemsInPack();
		if (itemSpaceRemaining) {
			sprintf(buttons[itemNumber + extraLineCount].text, "%s%s    You have room for %i more item%s.",
					grayColorEscapeSequence,
					(magicDetected ? "  " : ""),
					itemSpaceRemaining,
					(itemSpaceRemaining == 1 ? "" : "s"));
		} else {
			sprintf(buttons[itemNumber + extraLineCount].text, "%s%s    Your pack is full.",
					grayColorEscapeSequence,
					(magicDetected ? "  " : ""));	
		}
		maxLength = max(maxLength, (strLenWithoutEscapes(buttons[itemNumber + extraLineCount].text)));
		extraLineCount++;
		
		sprintf(buttons[itemNumber + extraLineCount].text, "%s%s -- press (a-z) for more info -- ",
				grayColorEscapeSequence,
				(magicDetected ? "  " : ""));
		maxLength = max(maxLength, (strLenWithoutEscapes(buttons[itemNumber + extraLineCount].text)));
		extraLineCount++;
	}
	if (equippedItemCount) {
		// Add a separator button to fill in the blank line between equipped and unequipped items.
		sprintf(buttons[itemNumber + extraLineCount].text, "      %s%s---",
				(magicDetected ? "  " : ""),
				grayColorEscapeSequence);
		buttons[itemNumber + extraLineCount].y = mapToWindowY(equippedItemCount);
		extraLineCount++;
	}
		
	for (i=0; i < itemNumber + extraLineCount; i++) {
		
		// Position the button.
		buttons[i].x = COLS - maxLength;
		
		// Pad the button label with space, so the button reaches to the right edge of the screen.
		m = strlen(buttons[i].text);
		for (j=buttons[i].x + strLenWithoutEscapes(buttons[i].text); j < COLS; j++) {
			buttons[i].text[m] = ' ';
			m++;
		}
		buttons[i].text[m] = '\0';
		
		// Display the button. This would be redundant with the button loop,
		// except that we want the display to stick around until we get rid of it.
		drawButton(&(buttons[i]), BUTTON_NORMAL, dbuf);
	}
	
	// Add invisible previous and next buttons, so up and down arrows can select items.
	// Previous
	buttons[itemNumber + extraLineCount + 0].flags = B_ENABLED; // clear everything else
	buttons[itemNumber + extraLineCount + 0].hotkey[0] = NUMPAD_8;
	buttons[itemNumber + extraLineCount + 0].hotkey[1] = UP_ARROW;
	// Next
	buttons[itemNumber + extraLineCount + 1].flags = B_ENABLED; // clear everything else
	buttons[itemNumber + extraLineCount + 1].hotkey[0] = NUMPAD_2;
	buttons[itemNumber + extraLineCount + 1].hotkey[1] = DOWN_ARROW;
	
	overlayDisplayBuffer(dbuf, rbuf);
	
	do {
		repeatDisplay = false;
		
		// Do the button loop.
		highlightItemLine = -1;
		overlayDisplayBuffer(rbuf, NULL);	// Remove the inventory display while the buttons are active,
											// since they look the same and we don't want their opacities to stack.
		
		highlightItemLine = buttonInputLoop(buttons,
											itemCount + extraLineCount + 2, // the 2 is for up/down hotkeys
											COLS - maxLength,
											mapToWindowY(0),
											maxLength,
											itemNumber + extraLineCount,
											&theEvent);
		if (highlightItemLine == itemNumber + extraLineCount + 0) {
			// Up key
			highlightItemLine = itemNumber - 1;
			theEvent.shiftKey = true;
		} else if (highlightItemLine == itemNumber + extraLineCount + 1) {
			// Down key
			highlightItemLine = 0;
			theEvent.shiftKey = true;
		}
		
		if (highlightItemLine >= 0) {
			theKey = itemList[highlightItemLine]->inventoryLetter;
			theItem = itemList[highlightItemLine];
		} else {
			theKey = ESCAPE_KEY;
		}
		
		// Was an item selected?
		if (highlightItemLine > -1 && (waitForAcknowledge || theEvent.shiftKey || theEvent.controlKey)) {
			
			do {
				// Yes. Highlight the selected item. Do this by changing the button color and re-displaying it.
				
				overlayDisplayBuffer(dbuf, NULL);
				
				//buttons[highlightItemLine].buttonColor = interfaceBoxColor;
				drawButton(&(buttons[highlightItemLine]), BUTTON_PRESSED, NULL);
				//buttons[highlightItemLine].buttonColor = black;
				
				if (theEvent.shiftKey || theEvent.controlKey || waitForAcknowledge) {
					// Display an information window about the item.
					actionKey = printCarriedItemDetails(theItem, max(2, mapToWindowX(DCOLS - maxLength - 42)), mapToWindowY(2), 40, includeButtons, NULL);
					
					overlayDisplayBuffer(rbuf, NULL); // remove the item info window
					
					if (actionKey == -1) {
						repeatDisplay = true;
						overlayDisplayBuffer(dbuf, NULL); // redisplay the inventory
					} else {
						restoreRNG;
						repeatDisplay = false;
						overlayDisplayBuffer(rbuf, NULL); // restore the original screen
					}
					
					switch (actionKey) {
						case APPLY_KEY:
							apply(theItem, true);
							break;
						case EQUIP_KEY:
							equip(theItem);
							break;
						case UNEQUIP_KEY:
							unequip(theItem);
							break;
						case DROP_KEY:
							drop(theItem);
							break;
						case THROW_KEY:
							throwCommand(theItem);
							break;
						case CALL_KEY:
							call(theItem);
							break;
						case UP_KEY:
							highlightItemLine = highlightItemLine - 1;
							if (highlightItemLine < 0) {
								highlightItemLine = itemNumber - 1;
							}
							break;
						case DOWN_KEY:
							highlightItemLine = highlightItemLine + 1;
							if (highlightItemLine >= itemNumber) {
								highlightItemLine = 0;
							}
							break;
						default:
							break;
					}
					
					if (actionKey == UP_KEY || actionKey == DOWN_KEY) {
						theKey = itemList[highlightItemLine]->inventoryLetter;
						theItem = itemList[highlightItemLine];
					} else if (actionKey > -1) {
						// Player took an action directly from the item screen; we're done here.
						restoreRNG;
						return 0;
					}
				}
			} while (actionKey == UP_KEY || actionKey == DOWN_KEY);
		}
	} while (repeatDisplay); // so you can get info on multiple items sequentially
	
	overlayDisplayBuffer(rbuf, NULL); // restore the original screen

	restoreRNG;
	return theKey;
}

short numberOfMatchingPackItems(unsigned long categoryMask,
								unsigned long requiredFlags, unsigned long forbiddenFlags,
								boolean displayErrors) {
	item *theItem;
	short matchingItemCount = 0;
	boolean lumenstone=false;
	
	if (packItems->nextItem == NULL) {
		if (displayErrors) {
			confirmMessages();
			message("Your pack is empty!", false);
		}
		return 0;
	}
	
	if (categoryMask & ENCHANT_POTION) {
		for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
			if (theItem->category & GEM) {
				lumenstone = true;
				break;
			}
		}
	}
	
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		
		if (((theItem->category & categoryMask)
			 || ((theItem->category & WEAPON) && theItem->kind == DART && (categoryMask & COATING_DARTS))
			 || ((theItem->category & SCROLL) && theItem->kind != SCROLL_DUPLICATION && (categoryMask & DUPLICATE_SCROLL))
			 || ((theItem->category & TOME) && theItem->kind != TOME_DUPLICATION && (categoryMask & DUPLICATE_TOME))
			 || ((theItem->category & POTION) && (theItem->kind != POTION_LIFE || lumenstone) && (categoryMask & ENCHANT_POTION))
			 || ((theItem->category & (POTION | SCROLL)) && theItem->quantity >= 2 && (categoryMask & COMBINE_2_POTION_OR_SCROLL))
			 || ((theItem->category & CHARM) && theItem->enchant1 < rogue.charming && (categoryMask & CHARMING_CHARM))
			 || ((theItem->category & TALISMAN) && (theItem->kind <= TALISMAN_MAX_ENCHANT || theItem->kind == TALISMAN_BALANCE) && (categoryMask & ENCHANT_TALISMAN))
//			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_CHAOS && (categoryMask & APPLY_TALISMAN))
			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_ALCHEMY && (categoryMask & APPLY_TALISMAN))			 
			 || ((theItem->category & TALISMAN) && theItem == rogue.talisman && theItem->kind == TALISMAN_SHAPE_CHANGING && (categoryMask & APPLY_TALISMAN))) &&
			(!(~(theItem->flags) & requiredFlags) || ((requiredFlags & ITEM_CURSED) && itemMagicChar(theItem) == BAD_MAGIC_CHAR)
			|| ((requiredFlags & ITEM_CAN_BE_IDENTIFIED) && (theItem->category & (WEAPON | ARMOR | SHIELD))
				&& rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY && !(theItem->flags & ITEM_RUNIC_IDENTIFIED))) &&
			!(theItem->flags & forbiddenFlags)) {
			
			matchingItemCount++;
		}
	}
	
	if (matchingItemCount == 0) {
		if (displayErrors) {
			confirmMessages();
			message("You have nothing suitable.", false);
		}
		return 0;
	}
	
	return matchingItemCount;
}

void updateEncumbrance() {
	short moveSpeed, attackSpeed;
	
	moveSpeed = player.info.movementSpeed;
	attackSpeed = player.info.attackSpeed;
	
	if (player.status[STATUS_HASTED_NOT_NEGATABLE]) {
		moveSpeed /= 4;
		attackSpeed /= 4;
	} else if (player.status[STATUS_HASTED]) {
		moveSpeed /= 2;
		attackSpeed /= 2;
	} else if (player.status[STATUS_SLOWED]) {
		moveSpeed *= 2;
		attackSpeed *= 2;
	}
	
	player.movementSpeed = moveSpeed;
	player.attackSpeed = attackSpeed;
	
	recalculateEquipmentBonuses();
}

short displayedArmorValue() {
    if (!rogue.armor || (rogue.armor->flags & ITEM_IDENTIFIED)) {
        return player.info.defense / 10;
    } else {
        return (short) (((armorTable[rogue.armor->kind].range.upperBound + armorTable[rogue.armor->kind].range.lowerBound) / 2) / 10 + strengthModifier(rogue.armor));
    }
}

void strengthCheck(item *theItem) {
	char buf1[COLS], buf2[COLS*2];
	short strengthDeficiency;
	
	updateEncumbrance();
	if (theItem) {
		if ((theItem->category & WEAPON) && theItem->strengthRequired > rogue.strength - player.weaknessAmount + rogue.mightBonus
			- (rogue.shield ? rogue.shield->kind * 2 + 1: 0)
			- (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0)) {
			strengthDeficiency = theItem->strengthRequired - (rogue.strength - player.weaknessAmount + rogue.mightBonus
															  - (rogue.shield ? rogue.shield->kind * 2 + 1: 0)
															  - (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0));
			strcpy(buf1, "");
			itemName(theItem, buf1, false, false, NULL);
			sprintf(buf2, "You can barely lift the %s; %i more strength would be ideal.", buf1, strengthDeficiency);
			message(buf2, false);
		}
		
		if ((theItem->category & ARMOR) && theItem->strengthRequired > rogue.strength - player.weaknessAmount + rogue.mightBonus
			- (rogue.shield ? rogue.shield->kind * 2 + 1: 0)
			- (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0)) {
			strengthDeficiency = theItem->strengthRequired - (rogue.strength - player.weaknessAmount + rogue.mightBonus
															  - (rogue.shield ? rogue.shield->kind * 2 + 1: 0)
															  - (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0));
			strcpy(buf1, "");
			itemName(theItem, buf1, false, false, NULL);
			sprintf(buf2, "You stagger under the weight of the %s; %i more strength would be ideal.",
					buf1, strengthDeficiency);
			message(buf2, false);
		}
			
		if (theItem->category & SHIELD) {
			if (theItem->strengthRequired > rogue.strength - player.weaknessAmount + rogue.mightBonus) {
				strengthDeficiency = theItem->strengthRequired - (rogue.strength - player.weaknessAmount + rogue.mightBonus);
				strcpy(buf1, "");
				itemName(theItem, buf1, false, false, NULL);
				sprintf(buf2, "You can barely hold the %s; %i more strength would be ideal.",
						buf1, strengthDeficiency);
				message(buf2, false);
			}
		}
	}
}

boolean canEquip(item *theItem) {
	item *previouslyEquippedItem = NULL;
	
	if (theItem->flags & ITEM_BROKEN) {
		return false;
	}
	if ((theItem->category & WEAPON) && (!rogue.talisman || rogue.talisman->kind != TALISMAN_SINISTER)) {
		previouslyEquippedItem = rogue.weapon;
	} else if (theItem->category & ARMOR) {
		previouslyEquippedItem = rogue.armor;
	} else if (theItem->category & SHIELD) {
		previouslyEquippedItem = rogue.shield;
	} else if (theItem->category & TALISMAN) {
		previouslyEquippedItem = rogue.talisman;
		if (theItem->kind == TALISMAN_THIRD_EYE) {
			previouslyEquippedItem = rogue.armor;
		}
	}
	if (previouslyEquippedItem && (previouslyEquippedItem->flags & ITEM_CURSED)) {
		return false; // already using a cursed item
	}
	if ((theItem->category & (WEAPON|TALISMAN)) && rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER && rogue.weapon && rogue.offhandWeapon) {
		return false;
	}
	
	if ((theItem->category & RING) && rogue.ringLeft && rogue.ringRight && (!rogue.talisman || rogue.talisman->kind != TALISMAN_THIRD_EYE || rogue.ringThird)) {
		return false;
	}
	
	if ((theItem->category & TALISMAN) && rogue.talisman && rogue.talisman->kind == TALISMAN_ALCHEMY && numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS) {
		return false;
	}
	
	if ((theItem->category & ARMOR) && rogue.talisman && rogue.talisman->kind == TALISMAN_THIRD_EYE) {
		return false;
	}
	
	return true;
}

// Will prompt for an item if none is given.
// Equips the item and records input if successful.
// Player's failure to select an item will result in failure.
// Failure does not record input.
void equip(item *theItem) {
	char buf1[COLS], buf2[COLS];
	unsigned char command[10];
	short c = 0;
	item *theItem2;
	
	command[c++] = EQUIP_KEY;
	if (!theItem) {
		theItem = promptForItemOfType((WEAPON|ARMOR|SHIELD|RING|TALISMAN), 0, ITEM_EQUIPPED, "Equip what? (a-z, shift for more info; or <esc> to cancel)", true);
	}
	if (theItem == NULL) {
		return;
	}
	
	command[c++] = theItem->inventoryLetter;
	
	if (theItem->category & (WEAPON|ARMOR|SHIELD|RING|TALISMAN)) {
		
		if (theItem->category & RING) {
			if (theItem->flags & ITEM_EQUIPPED) {
				confirmMessages();
				message("you are already wearing that ring.", false);
				return;
			} else if (rogue.ringLeft && rogue.ringRight && (!rogue.talisman || rogue.talisman->kind != TALISMAN_THIRD_EYE || rogue.ringThird)) {
				confirmMessages();
				sprintf(buf1, "You are already wearing %s rings; remove which first?", rogue.talisman && rogue.talisman->kind == TALISMAN_THIRD_EYE ? "three" : "two");
				
				theItem2 = promptForItemOfType((RING), ITEM_EQUIPPED, 0, buf1, true);
				if (!theItem2 || theItem2->category != RING || !(theItem2->flags & ITEM_EQUIPPED)) {
					if (theItem2) { // No message if canceled or did an inventory action instead.
						message("Invalid entry.", false);
					}
					return;
				} else {
					if (theItem2->flags & ITEM_CURSED) {
						itemName(theItem2, buf1, false, false, NULL);
						sprintf(buf2, "You can't remove your %s: it appears to be cursed.", buf1);
						confirmMessages();
						messageWithColor(buf2, &itemMessageColor, false);
						return;
					}
					unequipItem(theItem2, false);
					command[c++] = theItem2->inventoryLetter;
				}
			}
		}

		if ((theItem->category & WEAPON) && rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER) {
			if (theItem->flags & ITEM_EQUIPPED) {
				confirmMessages();
				message("you are already wielding that weapon.", false);
				return;
			} else if (rogue.weapon && rogue.offhandWeapon) {
				confirmMessages();
				
				theItem2 = promptForItemOfType((WEAPON), ITEM_EQUIPPED, 0,
											   "You are already wielding two weapons; remove which first?" , true);
				if (!theItem2 || theItem2->category != WEAPON || !(theItem2->flags & ITEM_EQUIPPED)) {
					if (theItem2) { // No message if canceled or did an inventory action instead.
						message("Invalid entry.", false);
					}
					return;
				} else {
					if (theItem2->flags & ITEM_CURSED) {
						itemName(theItem2, buf1, false, false, NULL);
						sprintf(buf2, "You can't remove your %s: it appears to be cursed.", buf1);
						confirmMessages();
						messageWithColor(buf2, &itemMessageColor, false);
						return;
					}
					unequipItem(theItem2, false);
					command[c++] = theItem2->inventoryLetter;
				}
			}
		}
		
		if (theItem->flags & ITEM_EQUIPPED) {
			confirmMessages();
			message("already equipped.", false);
			return;
		}
		
		if (theItem->flags & ITEM_BROKEN) {
			confirmMessages();
			messageWithColor("You can't; it is broken.", &itemMessageColor, false);
			return;
		}
		
		if ((theItem->category & (WEAPON|TALISMAN)) && rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER && rogue.weapon && rogue.offhandWeapon) {
			confirmMessages();
			messageWithColor("You must remove a weapon first.", &itemMessageColor, false);
			return;
		}
		
		if ((theItem->category & RING) && rogue.ringLeft && rogue.ringRight && (!rogue.talisman || rogue.talisman->kind != TALISMAN_THIRD_EYE || rogue.ringThird)) {
			confirmMessages();
			messageWithColor("You must remove a ring first.", &itemMessageColor, false);
			return;
		}
		
		if ((theItem->category & TALISMAN) && rogue.talisman && rogue.talisman->kind == TALISMAN_ALCHEMY && numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS) {
			confirmMessages();
			sprintf(buf1, "You can't; you must drop %i duplicate potion%s or scroll%s.",
					numberOfItemsInPackWithoutAlchemy() - MAX_PACK_ITEMS,
					numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS + 1 ? "s" : "",
					numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS + 1 ? "s" : "");
			messageWithColor(buf1, &itemMessageColor, false);
			return;
		}
		
		if ((theItem->category & ARMOR) && rogue.talisman && rogue.talisman->kind == TALISMAN_THIRD_EYE) {
			confirmMessages();
			messageWithColor("You must remove your armor or talisman first.", &itemMessageColor, false);
			return;
		}
		
		if (!canEquip(theItem)) {
			// equip failed because current item is cursed
			if (theItem->category & WEAPON) {
				itemName(rogue.weapon, buf1, false, false, NULL);
			} else if (theItem->category & ARMOR) {
				itemName(rogue.armor, buf1, false, false, NULL);
			} else if (theItem->category & SHIELD) {
				itemName(rogue.shield, buf1, false, false, NULL);
			} else if (theItem->category & TALISMAN) {
				if (theItem->kind == TALISMAN_THIRD_EYE) {
					itemName(rogue.armor, buf1, false, false, NULL);
				} else {
					itemName(rogue.talisman, buf1, false, false, NULL);
				}
			} else {
				sprintf(buf1, "one");
			}
			sprintf(buf2, "You can't; the %s you are using appears to be cursed.", buf1);
			confirmMessages();
			messageWithColor(buf2, &itemMessageColor, false);
			return;
		}
		command[c] = '\0';
		recordKeystrokeSequence(command);
		
		equipItem(theItem, false);
		
		itemName(theItem, buf2, true, true, NULL);
		sprintf(buf1, "Now %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
		confirmMessages();
		messageWithColor(buf1, &itemMessageColor, false);
		
		if (theItem->category & SHIELD || rogue.offhandWeapon == theItem) {
			strengthCheck(rogue.weapon);
			strengthCheck(rogue.armor);
		}
		strengthCheck(theItem);
		
		if (theItem->flags & ITEM_CURSED) {
			itemName(theItem, buf2, false, false, NULL);
			switch(theItem->category) {
				case WEAPON:
					sprintf(buf1, "you wince as your grip involuntarily tightens around your %s.", buf2);
					break;
				case ARMOR:
					sprintf(buf1, "your %s constricts around you painfully.", buf2);
					break;
				case SHIELD:
					sprintf(buf1, "your %s bites into your arm painfully.", buf2);
					break;
				case RING:
					sprintf(buf1, "your %s tightens around your finger painfully.", buf2);
					break;
				case TALISMAN:
					sprintf(buf1, "you greedily covet the %s.", buf2);
					break;
				default:
					sprintf(buf1, "your %s seizes you with a malevolent force.", buf2);
					break;
			}
			messageWithColor(buf1, &itemMessageColor, false);
		} else {
			theItem->flags |= (ITEM_KNOWN_NOT_CURSED);
		}
		playerTurnEnded();
	} else {
		confirmMessages();
		message("You can't equip that.", false);
	}
}

// Returns whether the given item is a key that can unlock the given location.
// An item qualifies if:
// (1) it's a key (has ITEM_IS_KEY flag),
// (2) its keyZ matches the depth, and
// (3) either its key (x, y) location matches (x, y), or its machine number matches the machine number at (x, y).
boolean keyMatchesLocation(item *theItem, short x, short y) {
	short i;
	
	if ((theItem->flags & ITEM_IS_KEY)
		&& theItem->keyZ == rogue.depthLevel) {
		
		for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].x || theItem->keyLoc[i].machine); i++) {
			if (theItem->keyLoc[i].x == x && theItem->keyLoc[i].y == y) {
				return true;
			} else if (theItem->keyLoc[i].machine == pmap[x][y].machineNumber) {
				return true;
			}
		}
	}
	return false;
}

item *keyInPackFor(short x, short y) {
	item *theItem;
	
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		if (keyMatchesLocation(theItem, x, y)) {
			return theItem;
		}
	}
	return NULL;
}

item *keyOnTileAt(short x, short y) {
	item *theItem;
	creature *monst;
	
	if ((pmap[x][y].flags & HAS_PLAYER)
		&& player.xLoc == x
		&& player.yLoc == y
		&& keyInPackFor(x, y)) {
		
		return keyInPackFor(x, y);
	}
	if (pmap[x][y].flags & HAS_ITEM) {
		theItem = itemAtLoc(x, y);
		if (keyMatchesLocation(theItem, x, y)) {
			return theItem;
		}
	}
	if (pmap[x][y].flags & HAS_MONSTER) {
		monst = monsterAtLoc(x, y);
		if (monst->carriedItem) {
			theItem = monst->carriedItem;
			if (keyMatchesLocation(theItem, x, y)) {
				return theItem;
			}
		}
	}
	return NULL;
}

void aggravateMonsters(short turns, short confusingTurns, boolean throughWalls, boolean driveMad, boolean ignoreAllies) {
	creature *monst;
	short i, j, **grid;
	unsigned long aggravatingAttacks;
	unsigned long aggravatingTraits;
	
	for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
		if (!throughWalls && !(pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW)) {
			continue;
		}
		if (ignoreAllies && monst->creatureState == MONSTER_ALLY) {
			continue;
		}
		
		if (monst->creatureState == MONSTER_SLEEPING) {
			wakeUp(monst);
		}
		if (monst->creatureState != MONSTER_ALLY && monst->leader != &player) {
			monst->creatureState = MONSTER_TRACKING_SCENT;
		}
		
		if (driveMad && !(monst->info.flags & MONST_INANIMATE)) {
			aggravatingAttacks = AGGRAVATED_ATTACKS & (monst->info.abilityFlags | monst->negatedAbilities);
			aggravatingTraits = AGGRAVATED_TRAITS & (monst->info.flags | monst->negatedBehaviors);

			if (aggravatingAttacks || aggravatingTraits) {
				if (turns) {
					monst->aggravatedAbilities |= aggravatingAttacks;
					monst->aggravatedBehaviors |= aggravatingTraits;
					monst->status[STATUS_AGGRAVATED] += turns;
					monst->maxStatus[STATUS_AGGRAVATED] = monst->status[STATUS_AGGRAVATED];
				} else {
					monst->negatedAbilities &= ~aggravatingAttacks;
					monst->negatedBehaviors &= ~aggravatingTraits;
					
					monst->status[STATUS_AGGRAVATED] = monst->maxStatus[STATUS_AGGRAVATED] = 1000;
				}
			}
			
			monst->info.abilityFlags &= ~(aggravatingAttacks);
			monst->info.flags &= ~(aggravatingTraits);
		}
		
		if (confusingTurns && !(monst->info.flags & MONST_INANIMATE)) {
			monst->status[STATUS_CONFUSED] += confusingTurns;
			monst->maxStatus[STATUS_CONFUSED] = monst->status[STATUS_CONFUSED];
		}
	}
	
	grid = allocDynamicGrid();
	fillDynamicGrid(grid, 0);
	
	calculateDistances(grid, player.xLoc, player.yLoc, T_PATHING_BLOCKER, NULL, false, true, false);
	
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			if (grid[i][j] >= 0 && grid[i][j] < 30000) {
				scentMap[i][j] = 0;
				addScentToCell(i, j, 2 * grid[i][j]);
			}
		}
	}
	
	freeDynamicGrid(grid);
}

// returns the number of entries in the list;
// also includes (-1, -1) as an additional terminus indicator after the end of the list
short getLineCoordinates(short listOfCoordinates[][2], short originLoc[2], short targetLoc[2]) {
	float targetVector[2], error[2];
	short largerTargetComponent, currentVector[2], previousVector[2], quadrantTransform[2], i;
	short currentLoc[2], previousLoc[2];
	short cellNumber = 0;
	
//#ifdef BROGUE_ASSERTS
//	assert(originLoc[0] != targetLoc[0] || originLoc[1] != targetLoc[1]);
//#else
	if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
		return 0;
	}
//#endif
	
	// Neither vector is negative. We keep track of negatives with quadrantTransform.
	for (i=0; i<= 1; i++) {
		targetVector[i] = targetLoc[i] - originLoc[i];
		if (targetVector[i] < 0) {
			targetVector[i] *= -1;
			quadrantTransform[i] = -1;
		} else {
			quadrantTransform[i] = 1;
		}
		currentVector[i] = previousVector[i] = error[i] = 0;
		currentLoc[i] = originLoc[i];
	}
	
	// normalize target vector such that one dimension equals 1 and the other is in [0, 1].
	largerTargetComponent = max(targetVector[0], targetVector[1]);
	targetVector[0] /= largerTargetComponent;
	targetVector[1] /= largerTargetComponent;
	
	do {
		for (i=0; i<= 1; i++) {
			
			previousLoc[i] = currentLoc[i];
			
			currentVector[i] += targetVector[i];
			error[i] += (targetVector[i] == 1 ? 0 : targetVector[i]);
			
			if (error[i] >= 0.5) {
				currentVector[i]++;
				error[i] -= 1;
			}
			
			currentLoc[i] = quadrantTransform[i]*currentVector[i] + originLoc[i];
			
			listOfCoordinates[cellNumber][i] = currentLoc[i];
		}
		
		//DEBUG printf("\ncell %i: (%i, %i)", cellNumber, listOfCoordinates[cellNumber][0], listOfCoordinates[cellNumber][1]);
		cellNumber++;
		
	} while (coordinatesAreInMap(currentLoc[0], currentLoc[1]));
	
	cellNumber--;
	
	listOfCoordinates[cellNumber][0] = listOfCoordinates[cellNumber][1] = -1; // demarcates the end of the list
	return cellNumber;
}

void getImpactLoc(short returnLoc[2], short originLoc[2], short targetLoc[2],
				  short maxDistance, boolean returnLastEmptySpace) {
	float targetVector[2], error[2];
	short largerTargetComponent, currentVector[2], previousVector[2], quadrantTransform[2], i;
	short currentLoc[2], previousLoc[2];
	creature *monst;
	
	monst = NULL;
	
	// Neither vector is negative. We keep track of negatives with quadrantTransform.
	for (i=0; i<= 1; i++) {
		targetVector[i] = targetLoc[i] - originLoc[i];
		if (targetVector[i] < 0) {
			targetVector[i] *= -1;
			quadrantTransform[i] = -1;
		} else {
			quadrantTransform[i] = 1;
		}
		currentVector[i] = previousVector[i] = error[i] = 0;
		currentLoc[i] = originLoc[i];
	}
	
	// normalize target vector such that one dimension equals 1 and the other is in [0, 1].
	largerTargetComponent = max(targetVector[0], targetVector[1]);
	targetVector[0] /= largerTargetComponent;
	targetVector[1] /= largerTargetComponent;
	
	do {
		for (i=0; i<= 1; i++) {
			
			previousLoc[i] = currentLoc[i];
			
			currentVector[i] += targetVector[i];
			error[i] += (targetVector[i] == 1 ? 0 : targetVector[i]);
			
			if (error[i] >= 0.5) {
				currentVector[i]++;
				error[i] -= 1;
			}
			
			currentLoc[i] = quadrantTransform[i]*currentVector[i] + originLoc[i];
		}
		
		if (!coordinatesAreInMap(currentLoc[0], currentLoc[1])) {
			break;
		}
		
		if (pmap[currentLoc[0]][currentLoc[1]].flags & HAS_MONSTER) {
			monst = monsterAtLoc(currentLoc[0], currentLoc[1]);
		}
		
	} while ((!(pmap[currentLoc[0]][currentLoc[1]].flags & HAS_MONSTER)
			  || !monst
			  || (monst->status[STATUS_INVISIBLE] || (monst->bookkeepingFlags & MONST_SUBMERGED)))
			 && !(pmap[currentLoc[0]][currentLoc[1]].flags & HAS_PLAYER)
			 && !cellHasTerrainFlag(currentLoc[0], currentLoc[1], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
			 && max(currentVector[0], currentVector[1]) <= maxDistance);
	
	if (returnLastEmptySpace) {
		returnLoc[0] = previousLoc[0];
		returnLoc[1] = previousLoc[1];
	} else {
		returnLoc[0] = currentLoc[0];
		returnLoc[1] = currentLoc[1];
	}
}

boolean tunnelize(short x, short y) {
	enum dungeonLayers layer;
	boolean didSomething = false;
	creature *monst;
	
	if (pmap[x][y].flags & IMPREGNABLE) {
		return false;
	}
	
	freeCaptivesEmbeddedAt(x, y);
	
	for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
		if (tileCatalog[pmap[x][y].layers[layer]].flags & (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) {
			pmap[x][y].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
			didSomething = true;
		}
	}
	if (didSomething) {
		spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_TUNNELIZE], true, false);
		if (pmap[x][y].flags & HAS_MONSTER) {
			// Kill turrets and sentinels if you tunnelize them.
			monst = monsterAtLoc(x, y);
			if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
				inflictDamage(monst, monst->currentHP, NULL);
			}
		}
	}
	return didSomething;
}

void negate(creature *monst, short turns) {
	unsigned long negatingAttacks = NEGATABLE_ATTACKS & (monst->info.abilityFlags | monst->aggravatedAbilities);
	unsigned long negatingTraits = NEGATABLE_TRAITS & (monst->info.flags | monst->aggravatedBehaviors);
	
	if (monst->info.flags & MONST_DIES_IF_NEGATED) {
		char buf[DCOLS * 3], monstName[DCOLS];
		monsterName(monstName, monst, true);
		if ((monst->info.flags & MONST_INANIMATE) && (monst->status[STATUS_LEVITATING])) {
			sprintf(buf, "%s dissipates into thin air", monstName);
		} else {
			sprintf(buf, "%s falls to the ground, lifeless", monstName);
		}
		killCreature(monst, false);
		combatMessage(buf, messageColorFromVictim(monst));
	} else {
		if (negatingAttacks || negatingTraits) {
			if (turns) {
				monst->negatedAbilities |= negatingAttacks;
				monst->negatedBehaviors |= negatingTraits;
				monst->status[STATUS_NEGATED] += turns;
				monst->maxStatus[STATUS_NEGATED] = monst->status[STATUS_NEGATED];			
			} else {
				monst->aggravatedAbilities &= ~negatingAttacks;
				monst->aggravatedBehaviors &= ~negatingTraits;
				// mark monster as negated if the abilities of the underlying race have been negated) {
				if ((monsterCatalog[monst->info.monsterID].flags & NEGATABLE_TRAITS) != 0 || (monsterCatalog[monst->info.monsterID].abilityFlags & NEGATABLE_ATTACKS) != 0) {
					monst->status[STATUS_NEGATED] = monst->maxStatus[STATUS_NEGATED] = 1000;
				}
			}
		}
		// works on inanimates
		monst->info.abilityFlags &= ~negatingAttacks; // negated monsters lose most special abilities
		if (negatingTraits & MONST_IMMUNE_TO_FIRE || monst->status[STATUS_IMMUNE_TO_FIRE] < 1000) {
			monst->status[STATUS_IMMUNE_TO_FIRE] = 0;
		}
		monst->status[STATUS_SLOWED] = 0;
		monst->status[STATUS_HASTED] = 0;
		monst->status[STATUS_CONFUSED] = 0;
		monst->status[STATUS_ENTRANCED] = 0;
		monst->status[STATUS_ENTRANCED_BY_WAND] = 0;
		monst->status[STATUS_DISCORDANT] = 0;
		monst->protectionAmount = 0;
		monst->status[STATUS_SHIELDED] = 0;
		if (negatingTraits & MONST_INVISIBLE || monst->status[STATUS_INVISIBLE] < 1000) {
			monst->status[STATUS_INVISIBLE] = 0;
		}
		monst->status[STATUS_REFLECTIVE] = 0;
		monst->status[STATUS_MARKED] = 0;
		monst->status[STATUS_BLESSED] = 0;
		if (monst->status[STATUS_DOMINATED]) {
			unAlly(monst);
			monst->status[STATUS_DOMINATED] = 0;
		}
		if (monst->status[STATUS_POLYMORPHED]) {
			monst->status[STATUS_POLYMORPHED] = 0;
			polymorph(monst, monst->polymorphedFrom, 0);
		}
		if (monst == &player) {
			monst->status[STATUS_TELEPATHIC] = min(monst->status[STATUS_TELEPATHIC], 1);
			monst->status[STATUS_MAGICAL_FEAR] = min(monst->status[STATUS_MAGICAL_FEAR], 1);
			monst->status[STATUS_LEVITATING] = min(monst->status[STATUS_LEVITATING], 1);
			if (monst->status[STATUS_DARKNESS]) {
				monst->status[STATUS_DARKNESS] = 0;
				updateMinersLightRadius();
				updateVision(true);
			}
		} else {
			monst->status[STATUS_TELEPATHIC] = 0;
			monst->status[STATUS_MAGICAL_FEAR] = 0;
			if (negatingTraits & MONST_FLIES || monst->status[STATUS_LEVITATING] < 1000) {
				monst->status[STATUS_LEVITATING] = 0;
			}
		}
		if (!monst->status[STATUS_HASTED]) {
			monst->movementSpeed = monst->info.movementSpeed;
			monst->attackSpeed = monst->info.attackSpeed;
		}
		if (monst != &player && (monst->info.flags & NEGATABLE_TRAITS)) {
			if ((monst->info.flags & MONST_FIERY) && monst->status[STATUS_BURNING]) {
				extinguishFireOnCreature(monst);
			}
			monst->info.flags &= ~negatingTraits;
			refreshDungeonCell(monst->xLoc, monst->yLoc);
			refreshSideBar(-1, -1, false);
		}
		applyInstantTileEffectsToCreature(monst); // in case it should immediately die or fall into a chasm
	}
}

short monsterAccuracyAdjusted(const creature *monst) {
    short retval = monst->info.accuracy * pow(WEAPON_ENCHANT_ACCURACY_FACTOR, 2.5 * (float) (monst->strengthAmount - monst->weaknessAmount));
    return max(retval, 0);
}

float monsterDamageAdjustmentAmount(const creature *monst) {
    if (monst == &player) {
        return 1.0; // Handled through player strength routines elsewhere.
    } else {
        return pow(WEAPON_ENCHANT_DAMAGE_FACTOR, 2.5 * (float) (monst->strengthAmount - monst->weaknessAmount));
    }
}

short monsterDefenseAdjusted(const creature *monst) {
    short retval = monst->info.defense + 25 * monst->strengthAmount - 25 * monst->weaknessAmount;
    return max(retval, 0);
}

// Adds one to the creature's weakness, sets the weakness status duration to maxDuration.
void weaken(creature *monst, short maxDuration) {
	if (monst->status[STATUS_STRONGER]) {
		monst->strengthAmount--;
		if (!monst->strengthAmount) {
			monst->status[STATUS_STRONGER] = 0;
		}
	} else if (monst->weaknessAmount < 10) {
        monst->weaknessAmount++;
    }
	if (monst->weaknessAmount) {
		monst->status[STATUS_WEAKENED] = max(monst->status[STATUS_WEAKENED], maxDuration);
		monst->maxStatus[STATUS_WEAKENED] = monst->status[STATUS_WEAKENED];
	}
	if (monst == &player) {
        messageWithColor("your muscles weaken as an enervating toxin fills your veins.", &badMessageColor, false);
		strengthCheck(rogue.weapon);
		strengthCheck(rogue.armor);
	}
}

// True if the creature polymorphed; false if not. Set kind = -1 to pick a random monster.
boolean polymorph(creature *monst, short monsterID, short turns) {
	short previousDamageTaken;
	float healthFraction;
	
	if (monst == &player || (monst->info.flags & MONST_INANIMATE)) {
		return false; // Sorry, this is not Nethack.
	}
	
	if (turns) {
		if (!monst->polymorphedFrom) {
			monst->polymorphedFrom = monst->info.monsterID;
		} else {
			monsterID = monst->polymorphedFrom;
			monst->polymorphedFrom = 0;
		}
		monst->status[STATUS_POLYMORPHED] += turns;
		monst->maxStatus[STATUS_POLYMORPHED] = monst->status[STATUS_POLYMORPHED];
	}
	
	if (!turns || monst->status[STATUS_DOMINATED]) { // prevent perma-dominated allies
		unAlly(monst); // Sorry, no cheap permanent dragon allies.
	}
	
	healthFraction = monst->currentHP / monst->info.maxHP;
	previousDamageTaken = monst->info.maxHP - monst->currentHP;
	
	if (monsterID >= 0) {
		monst->info = monsterCatalog[monsterID]; // Presto unchange-o!
	} else do {
		monst->info = monsterCatalog[rand_range(1, NUMBER_MONSTER_KINDS - 1)]; // Presto change-o!
	} while (monst->info.flags & MONST_INANIMATE); // Can't turn something into an inanimate object.
	
	if (monst->polymorphedFrom) {
		monst->maxStatus[STATUS_POLYMORPHED] = monst->status[STATUS_POLYMORPHED] = turns;
	}
	
    monst->info.turnsBetweenRegen *= 1000;
	monst->currentHP = max(1, max(healthFraction * monst->info.maxHP, monst->info.maxHP - previousDamageTaken));
	
	monst->movementSpeed = monst->info.movementSpeed;
	monst->attackSpeed = monst->info.attackSpeed;
	if (monst->status[STATUS_HASTED]) {
		monst->movementSpeed /= 2;
		monst->attackSpeed /= 2;
	}
	if (monst->status[STATUS_SLOWED]) {
		monst->movementSpeed *= 2;
		monst->attackSpeed *= 2;
	}
	
	clearStatus(monst);
	
	if (monst->info.flags & MONST_FIERY) {
		monst->status[STATUS_BURNING] = monst->maxStatus[STATUS_BURNING] = 1000; // won't decrease
	}
	if (monst->info.flags & MONST_FLIES) {
		monst->status[STATUS_LEVITATING] = monst->maxStatus[STATUS_LEVITATING] = 1000; // won't decrease
	}
	if (monst->info.flags & MONST_IMMUNE_TO_FIRE) {
		monst->status[STATUS_IMMUNE_TO_FIRE] = monst->maxStatus[STATUS_IMMUNE_TO_FIRE] = 1000; // won't decrease
	}
	if (monst->info.flags & MONST_INVISIBLE) {
		monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = 1000; // won't decrease
	}
	monst->status[STATUS_NUTRITION] = monst->maxStatus[STATUS_NUTRITION] = 1000;
	
	if (monst->bookkeepingFlags & MONST_CAPTIVE) {
		demoteMonsterFromLeadership(monst);
		monst->creatureState = MONSTER_TRACKING_SCENT;
		monst->bookkeepingFlags &= ~MONST_CAPTIVE;
	}
	
	monst->ticksUntilTurn = max(monst->ticksUntilTurn, 101);
	
	refreshDungeonCell(monst->xLoc, monst->yLoc);
	flashMonster(monst, boltColors[BOLT_POLYMORPH], 100);
	return true;
}

void slow(creature *monst, short turns) {
	if (!(monst->info.flags & MONST_INANIMATE) && !monst->status[STATUS_HASTED_NOT_NEGATABLE]) {
		monst->status[STATUS_SLOWED] = monst->maxStatus[STATUS_SLOWED] = turns;
		monst->status[STATUS_HASTED] = 0;
		if (monst == &player) {
			updateEncumbrance();
			message("you feel yourself slow down.", false);
		} else {
			monst->movementSpeed = monst->info.movementSpeed * 2;
			monst->attackSpeed = monst->info.attackSpeed * 2;
		}
	}
}

void haste(creature *monst, short turns, boolean notNegatable) {
	if (monst && !(monst->info.flags & MONST_INANIMATE)) {
		monst->status[STATUS_SLOWED] = 0;
		monst->status[STATUS_HASTED] = max(monst->status[STATUS_HASTED], turns);		
		monst->maxStatus[STATUS_HASTED] = monst->status[STATUS_HASTED];
		if (notNegatable) {
			monst->status[STATUS_HASTED_NOT_NEGATABLE] = monst->maxStatus[STATUS_HASTED_NOT_NEGATABLE] = turns;
		}
		if (monst == &player) {
			updateEncumbrance();
			message("you feel yourself speed up.", false);
		} else {
			monst->movementSpeed = monst->info.movementSpeed / 2;
			monst->attackSpeed = monst->info.attackSpeed / 2;
		}
	}
}

void heal(creature *monst, short percent) {	
	char buf[COLS], monstName[COLS];
	monst->currentHP = min(monst->info.maxHP, monst->currentHP + percent * monst->info.maxHP / 100);
	if (canDirectlySeeMonster(monst) && monst != &player) {
		monsterName(monstName, monst, true);
		sprintf(buf, "%s looks healthier", monstName);
		combatMessage(buf, NULL);
	}
}

void makePlayerTelepathic(short duration, boolean mark) {
    creature *monst;
    
    player.status[STATUS_TELEPATHIC] = player.maxStatus[STATUS_TELEPATHIC] = duration;
    for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
		if (mark && !monst->status[STATUS_MARKED]) {
			monst->status[STATUS_MARKED] = monst->maxStatus[STATUS_MARKED] = 1000;
			rogue.markedMonsters++;
		}
        refreshDungeonCell(monst->xLoc, monst->yLoc);
    }
    if (monsters->nextCreature == NULL) {
        message("you can somehow tell that you are alone on this level at the moment.", false);
    } else {
        message("you can somehow feel the presence of other creatures' minds!", false);
    }
}

void rechargeItems(unsigned long categories, boolean overcharge) {
    item *tempItem;
    short x, y, z, i, categoryCount;
    char buf[DCOLS * 3];
    
    x = y = z = 0; // x counts staffs, y counts wands, z counts charms
    for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
        if (tempItem->category & categories & STAFF) {
            x++;
			if (overcharge || !(tempItem->flags & ITEM_OVERCHARGED)) {
				tempItem->charges = tempItem->enchant1;
				tempItem->enchant2 = 500;
			}
			if (overcharge) {
				tempItem->charges += tempItem->enchant1;
				tempItem->flags |= (ITEM_OVERCHARGED);
			}
        }
        if (tempItem->category & categories & WAND) {
            y++;
            tempItem->charges++;
			if (overcharge) {
				tempItem->charges++;
			}
        }
        if (tempItem->category & categories & CHARM) {
            z++;
            tempItem->charges = 0;
			if (overcharge) {
				tempItem->flags |= (ITEM_OVERCHARGED);
			}
        }
    }
    
    categoryCount = (x ? 1 : 0) + (y ? 1 : 0) + (z ? 1 : 0);
    
    if (categoryCount) {
        i = 0;
        strcpy(buf, "a surge of energy courses through your pack, recharging your ");
        if (x) {
            i++;
            strcat(buf, x == 1 ? "staff" : "staffs");
            if (i == categoryCount - 1) {
                strcat(buf, " and ");
            } else if (i <= categoryCount - 2) {
                strcat(buf, ", ");
            }
        }
        if (y) {
            i++;
            strcat(buf, y == 1 ? "wand" : "wands");
            if (i == categoryCount - 1) {
                strcat(buf, " and ");
            } else if (i <= categoryCount - 2) {
                strcat(buf, ", ");
            }
        }
        if (z) {
            strcat(buf, z == 1 ? "charm" : "charms");
        }
        strcat(buf, ".");
        message(buf, false);
    } else {
        message("a surge of energy courses through your pack, but nothing happens.", false);
    }
}

void causeFear(const char *emitterName, boolean throughWalls, boolean ignoreAllies) {
    creature *monst;
    short numberOfMonsters = 0;
    char buf[DCOLS*3], mName[DCOLS];
    
    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
		if (ignoreAllies && monst->creatureState == MONSTER_ALLY) {
			continue;
		}
        if ((throughWalls || pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW)
            && monst->creatureState != MONSTER_FLEEING
            && !(monst->info.flags & MONST_INANIMATE)) {
            
            monst->status[STATUS_MAGICAL_FEAR] = monst->maxStatus[STATUS_MAGICAL_FEAR] = rand_range(150, 225);
            monst->creatureState = MONSTER_FLEEING;
            chooseNewWanderDestination(monst);
            if (canSeeMonster(monst)) {
                numberOfMonsters++;
                monsterName(mName, monst, true);
            }
        }
    }
    if (numberOfMonsters > 1) {
        sprintf(buf, "%s emits a brilliant flash of red light, and the monsters flee!", emitterName);
    } else if (numberOfMonsters == 1) {
        sprintf(buf, "%s emits a brilliant flash of red light, and %s flees!", emitterName, mName);
    } else {
        sprintf(buf, "%s emits a brilliant flash of red light!", emitterName);
    }
    message(buf, false);
    lightFlash(&redFlashColor, 0, IN_FIELD_OF_VIEW, 15, DCOLS, player.xLoc, player.yLoc);
}

void negateItem(item *theItem) {
	theItem->flags &= ~(ITEM_MAGIC_DETECTED | ITEM_CURSED);
	switch (theItem->category) {
		case WEAPON:
		case ARMOR:
		case SHIELD:
			theItem->enchant1 = theItem->enchant2 = theItem->charges = 0;
			theItem->flags &= ~(ITEM_RUNIC | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED | ITEM_PROTECTED);
			identify(theItem);
			break;
		case STAFF:
			theItem->charges = 0;
			break;
		case WAND:
			theItem->charges = 0;
			theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
			break;
		case RING:
		case TALISMAN:
			theItem->enchant1 = 0;
			theItem->flags |= ITEM_IDENTIFIED; // Reveal that it is (now) +0, but not necessarily which kind of ring it is.
			updateIdentifiableItems();
		case CHARM:
			theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
			break;
		default:
			break;
	}	
}

void negationBlast(const char *emitterName, boolean throughWalls, boolean onlyCursed, boolean ignoreAllies) {
    creature *monst, *nextMonst;
    item *theItem;
    char buf[DCOLS];
    
    sprintf(buf, "%s emits a numbing torrent of anti-magic!", emitterName);
    messageWithColor(buf, &itemMessageColor, false);
    lightFlash(&pink, 0, throughWalls ? (DISCOVERED | MAGIC_MAPPED) : (IN_FIELD_OF_VIEW), 15, DCOLS, player.xLoc, player.yLoc);
    negate(&player, 0);
    flashMonster(&player, &pink, 100);
    for (monst = monsters->nextCreature; monst != NULL;) {
        nextMonst = monst->nextCreature;
		if (ignoreAllies && monst->creatureState == MONSTER_ALLY) {
			continue;
		}
        if (throughWalls || pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW) {
            if (canSeeMonster(monst)) {
                flashMonster(monst, &pink, 100);
            }
            negate(monst, 0); // This can be fatal.
        }
        monst = nextMonst;
    }
    for (theItem = floorItems; theItem != NULL; theItem = theItem->nextItem) {
		if (onlyCursed && itemMagicChar(theItem) != BAD_MAGIC_CHAR) {
			continue;
		}
        if (throughWalls || pmap[theItem->xLoc][theItem->yLoc].flags & IN_FIELD_OF_VIEW) {
			negateItem(theItem);
        }
    }
}

void crystalize(short x, short y, short radius) {
	short i, j;
	creature *monst;
	char monstName[COLS];
	char buf[COLS];
	
	for (i=0; i<DCOLS; i++) {
		for (j=0; j < DROWS; j++) {
			if ((x - i) * (x - i) + (y - j) * (y - j) <= radius * radius
				&& !(pmap[i][j].flags & IMPREGNABLE)) {
				
				if (i == 0 || i == DCOLS - 1 || j == 0 || j == DROWS - 1) {
					pmap[i][j].layers[DUNGEON] = CRYSTAL_WALL; // don't dissolve the boundary walls
				} else if (tileCatalog[pmap[i][j].layers[DUNGEON]].flags & (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) {
					
					pmap[i][j].layers[DUNGEON] = FORCEFIELD;
					
					if (pmap[i][j].flags & HAS_MONSTER) {
						monst = monsterAtLoc(i, j);
						
						if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
							inflictDamage( monst, monst->currentHP, NULL);
						} else if ((monst->info.flags & MONST_REFLECT_4) && rand_percent(shatteringChance(radius - distanceFastApproximation(x, y, i, j)))) {
							monsterName(monstName, monst, true);
							if (canSeeMonster(monst)) {
								sprintf(buf,"the %s shatters!", monstName);
								combatMessage(buf, messageColorFromVictim(monst));
							}
							inflictDamage( monst, monst->currentHP, NULL);
						} else {
							freeCaptivesEmbeddedAt(i, j);
						}
					}
				}
			}
		}
	}
	updateVision(false);
	lightFlash(&forceFieldColor, 0, 0, radius, radius, x, y);
	displayLevel();
	refreshSideBar(-1, -1, false);
}

boolean imbueInvisibility(creature *monst, short duration, boolean hideDetails) {
    boolean autoID = false;
    
    if (monst && !(monst->info.flags & MONST_INANIMATE)) {
        if (monst == &player || monst->creatureState == MONSTER_ALLY) {
            autoID = true;
        }
        monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = duration;
        refreshDungeonCell(monst->xLoc, monst->yLoc);
        refreshSideBar(-1, -1, false);
        if (!hideDetails) {
            flashMonster(monst, boltColors[BOLT_INVISIBILITY], 100);	
        }
    }
    return autoID;
}

boolean magicMapping(short x, short y, short range, unsigned char machineNumber, unsigned long detectedFlags1, unsigned long detectedFlags2, unsigned long detectedFlags3) {
	short i, j, dx, dy;
	boolean hadEffect = false;
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			if (range) {
				dx = (x - i);
				dy = (y - j);
			}
			if (pmap[i][j].layers[DUNGEON] != GRANITE
				&& (!range || (dx*dx + dy*dy < range*range + range)) //rounding
				&& (!machineNumber || pmap[i][j].machineNumber == machineNumber)
				&& ((!detectedFlags1 && !detectedFlags2 && !detectedFlags3)
					|| cellHasTerrainFlag(i, j, detectedFlags1)
					|| cellHasTerrainFlag2(i, j, detectedFlags2)
					|| cellHasTerrainFlag3(i, j, detectedFlags3))) {
				if (!(pmap[i][j].flags & DISCOVERED)) {
					pmap[i][j].flags |= MAGIC_MAPPED;
				}
				hadEffect = true;
			}
		}
	}
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			if (range) {
				dx = (x - i);
				dy = (y - j);
			}
			if (cellHasTerrainFlag(i, j, T_IS_SECRET)
				&& (!range || (dx*dx + dy*dy < range*range + range)) //rounding
				&& (!machineNumber || pmap[i][j].machineNumber == machineNumber)
				&& ((!detectedFlags1 && !detectedFlags2 && !detectedFlags3)
					|| cellHasTerrainFlag(i, j, detectedFlags1)
					|| cellHasTerrainFlag2(i, j, detectedFlags2)
					|| cellHasTerrainFlag3(i, j, detectedFlags3))) {
				discover(i, j);
				pmap[i][j].flags |= MAGIC_MAPPED;
				pmap[i][j].flags &= ~STABLE_MEMORY;
				hadEffect = true;
			}
		}
	}
	return hadEffect;
}

void duplicateItem(item *theItem, short scale, boolean inInventory) {
	short i, j, x, y;
	item *tempItem, *temp2Item = NULL;
	char buf[2*COLS], buf2[COLS];
	short dropLoc[2];
	
	if (inInventory) {
		x = player.xLoc;
		y = player.yLoc;
	} else {
		x = theItem->xLoc;
		y = theItem->yLoc;
	}
	
	tempItem = generateItem(theItem->category, theItem->kind);
	tempItem->hiddenRunic = theItem->hiddenRunic;
	if (theItem->flags & ITEM_RUNIC) {
		tempItem->enchant2 = theItem->enchant2;
		tempItem->vorpalEnemy = theItem->vorpalEnemy;
		tempItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
	}
	if (theItem->flags & ITEM_CURSED) {
		tempItem->flags |= (ITEM_CURSED);
		if (tempItem->enchant1 > 0) {
			tempItem->enchant1 *= -1;
		} else if (tempItem->enchant1 == 0) {
			tempItem->enchant1 = -rand_range(1, 3);
		}
	} else {
		tempItem->flags &= ~(ITEM_CURSED);
		if (tempItem->enchant1 < 0) {
			tempItem->enchant1 *= -1;
		} else if (tempItem->enchant1 == 0 && theItem->enchant1 > 0) {
			tempItem->enchant1 = rand_range(1, 3);
		}
	}
	if (theItem->flags & ITEM_IS_KEY) {
		for (i = 0; i < KEY_ID_MAXIMUM; i++) {
			tempItem->keyLoc[i] = theItem->keyLoc[i];
		}
		tempItem->keyZ = theItem->keyZ;
		tempItem->keyAdoptedBy = theItem->keyAdoptedBy;
	}
	// Most scrolls and potions result in multiple items, except where this could be abusable
	if (((theItem->category & (POTION | ELIXIR)) != 0 && theItem->kind != POTION_STRENGTH && theItem->kind != POTION_LIFE)
		|| ((theItem->category & (SCROLL | TOME)) != 0 && theItem->kind != SCROLL_ENCHANTING && theItem->kind != SCROLL_DUPLICATION)) {
		if (magicCharDiscoverySuffix(theItem->category, theItem->kind) < 0) {
			j = rand_range(3, 4);
		} else {
			j = 2;
		}
	} else {
		j = 1;
	}
	j *= scale;
	if (j < 1) { // use scale = 0 as a proxy for give me just one item
		j = 1;
	}
	for (i = 0, theItem = tempItem; i < j; i++) {
		theItem = generateItem(ALL_ITEMS, -1);
		*theItem = *tempItem; // Make a copy
		if (inInventory && (numberOfItemsInPack() < MAX_PACK_ITEMS || theItem->category & GOLD || itemWillStackWithPack(theItem))) {
			temp2Item = addItemToPack(theItem);
		}
		else if (inInventory && !itemAtLoc(player.xLoc, player.yLoc)) {
			dropItem(theItem);
			itemName(theItem, buf, false, true, NULL);
			sprintf(buf2, "you drop %s.", buf);
			messageWithColor(buf2, &itemMessageColor, false);
		} else {
			getQualifyingLocNear(dropLoc, x, y, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
			placeItem(theItem, dropLoc[0], dropLoc[1]);
			refreshDungeonCell(dropLoc[0], dropLoc[1]);
			itemName(theItem, buf, false, true, NULL);
			sprintf(buf2, "you drop %s.", buf);
			messageWithColor(buf2, &itemMessageColor, false);
		}
	}
	if (temp2Item) {
		itemName(temp2Item, buf, false, true, NULL);
		sprintf(buf2, "you now have %s (%c).", buf, temp2Item->inventoryLetter);
		messageWithColor(buf2, &itemMessageColor, false);
	}	
}

void summonMonstersAroundPlayer(short alliedMonsters, short maxMonsters, short turns) {
	short i, j, x, y, numberOfMonsters = 0;
	creature *monst;
	
	for (j=0; j<25 && numberOfMonsters < 3; j++) {
		for (i=0; i<8; i++) {
			x = player.xLoc + nbDirs[i][0];
			y = player.yLoc + nbDirs[i][1];
			if (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY) && !(pmap[x][y].flags & HAS_MONSTER)
				&& rand_percent(10) && (numberOfMonsters < maxMonsters)) {
				monst = spawnHorde(0, x, y, (HORDE_LEADER_CAPTIVE | HORDE_NO_PERIODIC_SPAWN | HORDE_IS_SUMMONED | HORDE_MACHINE_ONLY), 0);
				if (monst) {
					// refreshDungeonCell(x, y);
					// monst->creatureState = MONSTER_TRACKING_SCENT;
					// monst->ticksUntilTurn = player.movementSpeed;
					if (numberOfMonsters < alliedMonsters) {
						becomeAllyWith(monst);
						monst->status[STATUS_DOMINATED] = monst->maxStatus[STATUS_DOMINATED] = turns;
					}
					wakeUp(monst);
					fadeInMonster(monst);
					numberOfMonsters++;
				}
			}
		}
	}
	if (numberOfMonsters > 1) {
		message("the fabric of space ripples, and monsters appear!", false);
	} else if (numberOfMonsters == 1) {
		message("the fabric of space ripples, and a monster appears!", false);
	} else {
		message("the fabric of space boils violently around you, but nothing happens.", false);
	}
}

boolean projectileReflects(creature *attacker, creature *defender) {	
	short prob, netReflectionLevel = 0;
	
	// immunity armor always reflects its vorpal enemy's projectiles
	if (defender == &player && rogue.armor && (((rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_IMMUNITY)
											   || (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
												   && (rogue.armor->hiddenRunicEnchantsRequired <= 0) && rogue.armor->hiddenRunic == A_IMMUNITY))
		&& attacker && rogue.armor->vorpalEnemy == attacker->info.slayID
		&& (!attacker || monstersAreEnemies(attacker, defender))) {
		return true;
	}
	
	if (defender == &player && rogue.shield && (((rogue.shield->flags & ITEM_RUNIC) && rogue.shield->enchant2 == A_IMMUNITY)
												|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
													&& (rogue.shield->hiddenRunicEnchantsRequired <= 0) && rogue.shield->hiddenRunic == A_IMMUNITY))
		&& attacker && rogue.shield->vorpalEnemy == attacker->info.slayID) {
		return true;
	}

	if (defender && (defender->status[STATUS_REFLECTIVE])) {
		return true;
	}
	
	if (defender == &player && rogue.armor && (((rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_REFLECTION)
											   || (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
												   && (rogue.armor->hiddenRunicEnchantsRequired <= 0) && rogue.armor->hiddenRunic == A_REFLECTION))) {
		netReflectionLevel += (short) netEnchant(rogue.armor);
	}
	
	if (defender == &player && rogue.shield && (((rogue.shield->flags & ITEM_RUNIC) && rogue.shield->enchant2 == A_REFLECTION)
												|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
													&& (rogue.shield->hiddenRunicEnchantsRequired) && rogue.shield->hiddenRunic == A_REFLECTION))) {
		netReflectionLevel += (short) netEnchant(rogue.shield);
	}
	
	if (defender && (defender->info.flags & MONST_REFLECT_4)) {
        if (defender->info.flags & MONST_ALWAYS_USE_ABILITY) {
            return true;
        }
		netReflectionLevel += 4;
	}
	
	if (netReflectionLevel <= 0) {
		return false;
	}
	
	prob = reflectionChance(netReflectionLevel);
	
	return rand_percent(prob);
}

// returns the path length of the reflected path, alters listOfCoordinates to describe reflected path
short reflectBolt(short targetX, short targetY, short listOfCoordinates[][2], short kinkCell, boolean retracePath) {
	short k, target[2], origin[2], newPath[DCOLS][2], newPathLength, failsafe, finalLength;
	boolean needRandomTarget;
	
	needRandomTarget = (targetX < 0 || targetY < 0
						|| (targetX == listOfCoordinates[kinkCell][0] && targetY == listOfCoordinates[kinkCell][1]));
	
	if (retracePath) {
		// if reflecting back at caster, follow precise trajectory until we reach the caster
		for (k = 1; k <= kinkCell && kinkCell + k < MAX_BOLT_LENGTH; k++) {
			listOfCoordinates[kinkCell + k][0] = listOfCoordinates[kinkCell - k][0];
			listOfCoordinates[kinkCell + k][1] = listOfCoordinates[kinkCell - k][1];
		}
		
		// Calculate a new "extension" path, with an origin at the caster, and a destination at
		// the caster's location translated by the vector from the reflection point to the caster.
		// 
		// For example, if the player is at (0,0), and the caster is at (2,3), then the newpath
		// is from (2,3) to (4,6):
		// (2,3) + ((2,3) - (0,0)) = (4,6).
		
		origin[0] = listOfCoordinates[2 * kinkCell][0];
		origin[1] = listOfCoordinates[2 * kinkCell][1];
		target[0] = targetX + (targetX - listOfCoordinates[kinkCell][0]);
		target[1] = targetY + (targetY - listOfCoordinates[kinkCell][1]);
		newPathLength = getLineCoordinates(newPath, origin, target);
		
		for (k=0; k<=newPathLength; k++) {
			listOfCoordinates[2 * kinkCell + k + 1][0] = newPath[k][0];
			listOfCoordinates[2 * kinkCell + k + 1][1] = newPath[k][1];
		}
		finalLength = 2 * kinkCell + newPathLength + 1;
	} else {
		failsafe = 50;
		do {
			if (needRandomTarget) {
				// pick random target
				perimeterCoords(target, rand_range(0, 39));
				target[0] += listOfCoordinates[kinkCell][0];
				target[1] += listOfCoordinates[kinkCell][1];
			} else {
				target[0] = targetX;
				target[1] = targetY;
			}
			
			newPathLength = getLineCoordinates(newPath, listOfCoordinates[kinkCell], target);
			
			if (!cellHasTerrainFlag(newPath[0][0], newPath[0][1], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
				needRandomTarget = false;
			}
			
		} while (needRandomTarget && --failsafe);
		
		for (k = 0; k < newPathLength; k++) {
			listOfCoordinates[kinkCell + k + 1][0] = newPath[k][0];
			listOfCoordinates[kinkCell + k + 1][1] = newPath[k][1];
		}
		
		finalLength = kinkCell + newPathLength + 1;
	}
	
	listOfCoordinates[finalLength][0] = -1;
	listOfCoordinates[finalLength][1] = -1;
	return finalLength;
}

// Update stuff that promotes without keys so players can't abuse item libraries with blinking/haste shenanigans
void checkForMissingKeys(short x, short y) {
	short layer;

	if (cellHasTerrainFlag(x, y, T_PROMOTES_WITHOUT_KEY) && !keyOnTileAt(x, y)) {
		for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
			if (tileCatalog[pmap[x][y].layers[layer]].flags & T_PROMOTES_WITHOUT_KEY) {
				promoteTile(x, y, layer, false);
			}
		}
	}
	
	if (cellHasTerrainFlag2(x, y, T2_PROMOTES_WITHOUT_ENTRY) && !(pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER))) {
		for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
			if (tileCatalog[pmap[x][y].layers[layer]].flags2 & T2_PROMOTES_WITHOUT_ENTRY) {
				promoteTile(x, y, layer, false);
			}
		}
	}
}

void backUpLighting(short lights[DCOLS][DROWS][3]) {
	short i, j, k;
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			for (k=0; k<3; k++) {
				lights[i][j][k] = tmap[i][j].light[k];
			}
		}
	}
}

void restoreLighting(short lights[DCOLS][DROWS][3]) {
	short i, j, k;
	for (i=0; i<DCOLS; i++) {
		for (j=0; j<DROWS; j++) {
			for (k=0; k<3; k++) {
				tmap[i][j].light[k] = lights[i][j][k];
			}
		}
	}
}

// Attack blocked by the players shield.
// Range = 0 is adjacent.
boolean shieldBlocks(creature *defender, short range, short damage, boolean degrade)
{
	item *theShield;
	short chance = 0;
	short dropLoc[2];

	// Can't lift shield
	if (defender->status[STATUS_STUCK] || defender->status[STATUS_PARALYZED] || defender->status[STATUS_ENTRANCED]) {
		return false;
	}
	
	if (defender != &player) {
		
		// Can't defend with shield
		if (defender->bookkeepingFlags & MONST_CAPTIVE || defender->creatureState == MONSTER_SLEEPING) {
			return false;
		}
		
		if (defender->info.flags & MONST_SHIELD_BLOCKS) {
			chance = clamp(monsterDefenseAdjusted(defender)/2 + 3 * range, 0, min(100, 95 + range));
		}

		// Don't degrade monster shields. This makes ogres more powerful allies, which they need to be.

		return rand_percent(chance);
	}
	
	if (!rogue.shield) {
		return false;
	}
	
	chance = clamp(rogue.shield->shieldChance + 4 * netEnchant(rogue.shield) + 3 * range, 0, min(100, 95 + range));

	if (rand_percent(chance))
	{
		if (degrade && !(rogue.shield->flags & ITEM_PROTECTED)) {
			rogue.shield->enchant1--;
			rogue.shield->shieldBlows--;
			rogue.shield->shieldMinBlow--;
			combatMessage("your shield weakens!", &badMessageColor);
		}
		
		if (damage >= rogue.shield->shieldMinBlow && !(--rogue.shield->shieldBlows))
		{
			rogue.shield->flags |= ITEM_BROKEN;
			theShield = rogue.shield;
			unequipItem(theShield, true);
			removeItemFromChain(theShield, packItems);
			getQualifyingLocNear(dropLoc, player.xLoc, player.yLoc, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM | HAS_PLAYER), false, false);
			placeItem(theShield, dropLoc[0], dropLoc[1]);
			refreshDungeonCell(dropLoc[0], dropLoc[1]);
			messageWithColor("your shield is broken in two!", &badMessageColor, true);
		}
		
		return true;
	}
	
	return false;
}


#define MAX_SPARK_LENGTH	6
#define MAX_SPARKS			6


#define MAX_SPARK_LENGTH	6
#define MAX_SPARKS			6

// create a number of sparks which randomly shoot out from the origin
// used for when a lightning bolt grounds against a target in water, or
// for T2_IS_SPARKING terrain e.g. damaged capacitors
void spark(short originLoc[2], short sparks, short damage) {
	short listOfCoordinates[MAX_SPARK_LENGTH][2][MAX_SPARKS];
	boolean sparkStopped[MAX_SPARKS], sparkSkipped[MAX_SPARKS];
	boolean fastForward = false;
	boolean forceCapacitor;
	short i, j, k, x, y, dx, dy, layer, qualifyingCandidates, failsafe;
	char candidates[10][2];
	creature *monst = NULL;
	char buf[COLS], monstName[COLS];
	
	sparks = min(sparks, MAX_SPARKS);
	failsafe = 5;
	
	// Step one for all sparks is to choose a random direction and advance
	for (j = 0; j < sparks; j++)
	{
		sparkStopped[j] = false;
		sparkSkipped[j] = false;
		
		// start at the origin
		listOfCoordinates[0][0][j] = originLoc[0];
		listOfCoordinates[0][1][j] = originLoc[1];
		
		// pick a random direction at start. Ignore obstructed grids to allow spark to travel further near walls.
		do {
			k = rand_range(0, 7);
			
			// next step is randomly chosen direction
			x = listOfCoordinates[1][0][j] = originLoc[0] + nbDirs[k][0];
			y = listOfCoordinates[1][1][j] = originLoc[1] + nbDirs[k][1];
		} while (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) && --failsafe);
	}
	
	for (i = 1; i < MAX_SPARK_LENGTH; i++) {
		for (j = 0; j < sparks; j++) {
			if (sparkStopped[j]) {
				continue;
			}
			
			x = listOfCoordinates[i][0][j];
			y = listOfCoordinates[i][1][j];
		
			// annoying and causes visual bugs when changing level if visible with omniscience 
			if (/*rogue.playbackOmniscience ||*/ (pmap[x][y].flags & ANY_KIND_OF_VISIBLE)) {
				// Update the visual effect of the spark.
				hiliteCell(x, y, &lightningColor, 100/i, false);
			}
			if (!fastForward && ((pmap[x][y].flags & ANY_KIND_OF_VISIBLE)/* || rogue.playbackOmniscience*/)) {
				fastForward = rogue.playbackFastForward || pauseBrogue(10);
			}
			
			// damage monsters in grid. TODO: skip flying or levitating monsters
			if (pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER))
			{
				monst = monsterAtLoc(x, y);
				monsterName(monstName, monst, true);
				
				if (inflictDamage( monst, damage, &lightningColor)) {
					// killed monster
					if (player.currentHP <= 0) {
						gameOver("Killed by a wayward spark", true);
						return;
					}
					if (pmap[x][y].flags & IN_FIELD_OF_VIEW) {
						sprintf(buf, "a spark %s %s",
								((monst->info.flags & MONST_INANIMATE) ? "destroys" : "kills"),
								monstName);
						combatMessage(buf, messageColorFromVictim(monst));
					} else {
						sprintf(buf, "you hear %s %s", monstName, ((monst->info.flags & MONST_INANIMATE) ? "be destroyed" : "die"));
						combatMessage(buf, messageColorFromVictim(monst));
					}
				} else {
					// monster lives
					if (pmap[x][y].flags & IN_FIELD_OF_VIEW) {
						sprintf(buf, "a spark hits %s",
								monstName);
						combatMessage(buf, messageColorFromVictim(monst));
					}
				}
			}
			
			// Sparks promotes some grids
			if (cellHasTerrainFlag2(x, y, (T2_PROMOTES_WITH_LIGHTNING))) {
				for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
					if (tileCatalog[pmap[x][y].layers[layer]].flags2 & T2_PROMOTES_WITH_LIGHTNING) {						
						promoteTile(x, y, layer, false);
					}
				}
			}
			
			// is there an obstruction?
			if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) &&
				!cellHasTerrainFlag2(x, y, (T2_CONDUCTIVE_WALL))) {
				sparkStopped[j] = true;
				continue;
			}
			
			// are we jumping over an intervening grid, and therefore already have the next move?
			if (sparkSkipped[j]) {
				sparkSkipped[j] = false;
				continue;
			}
			
			// are we displaying the last grid we'll traverse, and therefore don't need to find the next move?
			if (i == MAX_SPARK_LENGTH - 1) {
				continue;
			}
			
			// look for next move
			qualifyingCandidates = 0;
			
			// have we found a capacitor
			forceCapacitor = false;
			
			// find next grid
			for (dx = -1; dx <= 1; dx++) {
				for (dy = -1; dy <= 1; dy++) {
					
					// we slide the window 1 grid in the direction we were moving from previously, which is why this math is 'unusual'
					x = dx + 2 * listOfCoordinates[i][0][j] - listOfCoordinates[i-1][0][j];
					y = dy + 2 * listOfCoordinates[i][1][j] - listOfCoordinates[i-1][1][j];
					
					if (!coordinatesAreInMap(x, y)) {
						continue;
					}
					
					// don't turn around
					if (x == listOfCoordinates[i-1][0][j] && y == listOfCoordinates[i-1][1][j]) {
						continue;
					}
					
					// don't jump at end of bolt
					if (i == MAX_SPARK_LENGTH - 2 && distanceDiagonal(listOfCoordinates[i][0][j], listOfCoordinates[i][1][j], x, y) > 1)
					{
						continue;
					}
					
					// don't target conductive grids through walls
					if (distanceDiagonal(listOfCoordinates[i][0][j], listOfCoordinates[i][1][j], x, y) > 1
						&& cellHasTerrainFlag((listOfCoordinates[i][0][j] + x)/2,
											  (listOfCoordinates[i][1][j] + y)/2, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
						&& !cellHasTerrainFlag2((listOfCoordinates[i][0][j] + x)/2,
												(listOfCoordinates[i][1][j] + y)/2, (T2_CONDUCTIVE_WALL | T2_CONDUCTIVE_FLOOR))) {
						continue;
					}
					
					// force hitting capacitor
					if (cellHasTerrainFlag2(x, y, (T2_CONDUCTIVE_WALL)) && !forceCapacitor) {
						dx = -1;
						dy = -1;
						
						qualifyingCandidates = 0;
						forceCapacitor = true;
					} else if (forceCapacitor) {
						continue;
					}
					
					// Candidates either have a monster in, or are conductive.
					if (cellHasTerrainFlag2(x, y, (T2_CONDUCTIVE_FLOOR | T2_CONDUCTIVE_WALL))) {
						candidates[qualifyingCandidates][0] = x;
						candidates[qualifyingCandidates][1] = y;
						
						qualifyingCandidates++;
					} else if (pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER)) {
						monst = monsterAtLoc(x, y);
						if (!monst->status[STATUS_LEVITATING]) {
							candidates[qualifyingCandidates][0] = x;
							candidates[qualifyingCandidates][1] = y;
							
							qualifyingCandidates++;
						}
					}
				}
			}
			
			if (!qualifyingCandidates) {
				sparkStopped[j] = true;
			} else {
				k = rand_range(0, qualifyingCandidates-1);
				
				// have chosen directly adjacent candidate?
				if (distanceDiagonal(listOfCoordinates[i][0][j], listOfCoordinates[i][1][j], candidates[k][0], candidates[k][1]) <= 1) {
					listOfCoordinates[i+1][0][j] = candidates[k][0];
					listOfCoordinates[i+1][1][j] = candidates[k][1];
				} else {
					if (i < MAX_SPARK_LENGTH - 2) {
						listOfCoordinates[i+2][0][j] = candidates[k][0];
						listOfCoordinates[i+2][1][j] = candidates[k][1];
					}
					
					// there are possibly better approaches here
					listOfCoordinates[i+1][0][j] = (candidates[k][0] + listOfCoordinates[i][0][j]) / 2;
					listOfCoordinates[i+1][1][j] = (candidates[k][1] + listOfCoordinates[i][1][j]) / 2;
					sparkSkipped[j] = true;
				}
			}
		}
	}	
}

// tunnelizes if we're tunnelling; create a worm burrow if we're firing an underworm bolt
void tunnelizeOrBurrow(short x, short y, short bolt)
{
	if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY))) {
		 if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_VISION)) && bolt == BOLT_UNDERWORM) {
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_WORM_TUNNEL_MARKER_ACTIVE], true, false);
		 } else {
			 tunnelize(x, y);
		 }
	}	
}

// returns the terrain best suited for planting in this grid 
short gridPlantingTerrain(short x, short y)
{	
	// we use a run of contiguous surface terrain types to represent each plant
	if (pmap[x][y].layers[SURFACE] >= GRASS && pmap[x][y].layers[SURFACE] <= HAY) {
		return pmap[x][y].layers[SURFACE];
	} else if ((pmap[x][y].layers[SURFACE] == FOLIAGE) || (pmap[x][y].layers[SURFACE] == TRAMPLED_FOLIAGE)) {
		return GRASS; // grows bloodwort
	} else if (pmap[x][y].layers[SURFACE] == DEAD_FOLIAGE) {
		return DEAD_GRASS; // grows stinkfruit
	} else if ((pmap[x][y].layers[SURFACE] == FUNGUS_FOREST) || (pmap[x][y].layers[SURFACE] == TRAMPLED_FUNGUS_FOREST)) {
		return LUMINESCENT_FUNGUS; // grows crimson cap
	} else if ((pmap[x][y].layers[SURFACE] == CREEPER_FUNGUS) || (pmap[x][y].layers[SURFACE] == TRAMPLED_CREEPER_FUNGUS)) {
		return LICHEN; // grows symbiotic fungus
	} else if ((pmap[x][y].layers[SURFACE] == MANDRAKE_ROOT_NATURE) || (pmap[x][y].layers[SURFACE] == MANDRAKE_ROOT_MACHINE)
			   || (pmap[x][y].layers[SURFACE] == MANDRAKE_SAC_NATURE) || (pmap[x][y].layers[SURFACE] == MANDRAKE_SAC_MACHINE)
			   || (pmap[x][y].layers[SURFACE] == DEAD_MANDRAKE_ROOT) || (pmap[x][y].layers[SURFACE] == SPLIT_MANDRAKE_SAC)) {
		return RED_BLOOD; // grows mandrake root
	} else if (pmap[x][y].layers[LIQUID] == DEEP_WATER) {
		return DEEP_WATER; // grows algae
	} else if (pmap[x][y].layers[LIQUID] == MUD) {
		return GRAY_FUNGUS; // grows dead man's ear
	}
	
	return 0;
}

void landMonster(short x, short y, creature *landingMonst) {
	creature *monst;
	short x2, y2;
	
	if (pmap[x][y].flags & HAS_MONSTER) { // We're blinking onto an area already occupied by a submerged monster.
		// Make sure we don't get the shooting monster by accident.
		landingMonst->xLoc = landingMonst->yLoc = -1; // Will be set back to the destination in a moment.
		monst = monsterAtLoc(x, y);
		findAlternativeHomeFor(monst, &x2, &y2, true);
		if (x2 >= 0) {
			// Found an alternative location.
			monst->xLoc = x2;
			monst->yLoc = y2;
			pmap[x][y].flags &= ~HAS_MONSTER;
			pmap[x2][y2].flags |= HAS_MONSTER;
		} else {
			// No alternative location?? Hard to imagine how this could happen.
			// Just bury the monster and never speak of this incident again.
			killCreature(monst, true);
			pmap[x][y].flags &= ~HAS_MONSTER;
			monst = NULL;
		}
	}
	pmap[x][y].flags |= (landingMonst == &player ? HAS_PLAYER : HAS_MONSTER);
	landingMonst->xLoc = x;
	landingMonst->yLoc = y;
	applyInstantTileEffectsToCreature(landingMonst);
	
	if (landingMonst == &player) {
		updateVision(true);
	}
}

// returns whether the bolt effect should autoID any staff or wand it came from, if it came from a staff or wand
boolean zap(short originLoc[2], short targetLoc[2], enum boltType bolt, short boltLevel, float power, unsigned long boltHasFlags) {
	enum dungeonLayers layer;

	short listOfCoordinates[MAX_BOLT_LENGTH][2];
	short i, j, k, x, y, x2, y2, numCells, boltDistance, boltLength, initialBoltLength, newLoc[2], lights[DCOLS][DROWS][3];
	short damage, pushDamage, hp, horde, duration, hitCount, numberOfReflectsLeft = 0;
	short originalBoltLevel = boltLevel;
	creature *monst = NULL, *pushedMonst = NULL, *shootingMonst, *hitList[8] = {NULL};
	char buf[COLS], monstName[COLS];
	boolean autoID = false;
	boolean fastForward = false;
	boolean alreadyReflected = false;
	boolean boltInView;
	color *boltColor;
    //color boltImpactColor;
	dungeonFeature feat;

#ifdef BROGUE_ASSERTS
	assert(originLoc[0] != targetLoc[0] || originLoc[1] != targetLoc[1]);
#else
	if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
		return false;
	}
#endif
	
	x = originLoc[0];
	y = originLoc[1];
	
	initialBoltLength = boltLength = (short) 5 * boltLevel * power;
    
	lightSource boltLights[initialBoltLength];
	color boltLightColors[initialBoltLength];
    
	numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc);
	
	shootingMonst = monsterAtLoc(originLoc[0], originLoc[1]);
	
	if (!(boltHasFlags & BOLT_HIDE_DETAILS)) {
		boltColor = boltColors[bolt];
	} else {
		boltColor = &gray;
	}
    
    //boltImpactColor = *boltColor;
    //applyColorScalar(&boltImpactColor, 5000);
	
	refreshSideBar(-1, -1, false);
    displayCombatText(); // To announce who fired the bolt while the animation plays.
	
	if (bolt == BOLT_BLINKING) {
		// short-range blinking allows you to push a monster ahead of you, open doors and trample foliage
		if (boltHasFlags & BOLT_DISTANCE) {
			zap(originLoc, newLoc, BOLT_FORCE, boltLevel, power, boltHasFlags);
		}
		if (cellHasTerrainFlag(listOfCoordinates[0][0], listOfCoordinates[0][1], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
			|| ((pmap[listOfCoordinates[0][0]][listOfCoordinates[0][1]].flags & (HAS_PLAYER | HAS_MONSTER))
				&& !(monsterAtLoc(listOfCoordinates[0][0], listOfCoordinates[0][1])->bookkeepingFlags & MONST_SUBMERGED))) {
				// shooting blink point-blank into an obstruction does nothing.
				return false;
		}
		if ((boltHasFlags & BOLT_SELECTIVE) && (cellHasTerrainFlag(targetLoc[0], targetLoc[1], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
			|| ((pmap[targetLoc[0]][targetLoc[1]].flags & (HAS_PLAYER | HAS_MONSTER))
				&& !(monsterAtLoc(targetLoc[0], targetLoc[1])->bookkeepingFlags & MONST_SUBMERGED)))) {
			// selecting an obstruction does nothing.
			return false;
		}
		pmap[originLoc[0]][originLoc[1]].flags &= ~(HAS_PLAYER | HAS_MONSTER);
		refreshDungeonCell(originLoc[0], originLoc[1]);
		checkForMissingKeys(originLoc[0], originLoc[1]);
	}

	// limit the distance the bolt travels: beckoning bolts have only beckon the target 50% of the way back if
	// they also have the BOLT_DISTANCE flag, but we don't limit the distance of the beckoning bolt beyond normal
	if ((boltHasFlags & BOLT_DISTANCE) && bolt != BOLT_BECKONING) {
		boltDistance = (short)boltLevel * power + 1;
	} else if (bolt == BOLT_BLINKING || bolt == BOLT_FORCE || ((bolt == BOLT_BECKONING || bolt == BOLT_TELEPORT || bolt == BOLT_MIRRORED_TOTEM) && (boltHasFlags & BOLT_FROM_STAFF))) {
		boltDistance = boltLevel * power * 2 + 1;
	}
	
	// record the last wall location to connect underworm tunnels
	if (bolt == BOLT_UNDERWORM) {
		x2 = -1;
		y2 = -1;
	}
	
	// crimson cap bolts pass through one wall/creature
	if (bolt == BOLT_CRIMSON_CAP) {
		boltHasFlags |= BOLT_X_RAY;
	}

	// precalculate damage to allow this to be used up by fire bolts and penetrating bolts
	damage = (short) boltDamage(boltLevel * power);

	// lightning and bouncing bolts reflect off walls
	if (boltHasFlags & BOLT_BOUNCES) {
		numberOfReflectsLeft++;
	}
	// only staff of lightning reflect natively
	if (bolt == BOLT_LIGHTNING && (boltHasFlags & (BOLT_FROM_STAFF | BOLT_FROM_WAND))) {
		numberOfReflectsLeft++;
	}
	if (numberOfReflectsLeft) {
		numberOfReflectsLeft++;
	}
	
	for (i=0; i<initialBoltLength; i++) {
		boltLightColors[i] = *boltColor;
		boltLights[i] = lightCatalog[BOLT_LIGHT_SOURCE];
		boltLights[i].lightColor = &boltLightColors[i];
		boltLights[i].lightRadius.lowerBound = boltLights[i].lightRadius.upperBound = 50 * (3 + boltLevel * 1.33) * (initialBoltLength - i) / initialBoltLength;
	}
	
	if (bolt == BOLT_TUNNELING && !cellHasTerrainFlag2(x, y, (T2_TUNNELIZE_IGNORES_GRID))) {
		tunnelize(originLoc[0], originLoc[1]);
	}
	
	backUpLighting(lights);
	boltInView = true;
	for (i=0; i<numCells; i++) {
		
		x = listOfCoordinates[i][0];
		y = listOfCoordinates[i][1];
		
		monst = monsterAtLoc(x, y);
		
		if (x == targetLoc[0] && y == targetLoc[1] && (!monst || bolt < BOLT_SENTRY)) {
			boltHasFlags &= ~(BOLT_SELECTIVE);
		}
		
		if (monst && (boltHasFlags & BOLT_CHAINING)) {
			monst->bookkeepingFlags |= MONST_BOLT_CHAINED; // prevent chained bolts getting hung up on submerged monsters
		}
		
		// Player travels inside the bolt when it is blinking or pushed.
		if ((bolt == BOLT_BLINKING && shootingMonst == &player) || (bolt == BOLT_FORCE && pushedMonst == &player)) {
			if (!i && rogue.impaleDirection >= 0) {
				x2 = player.xLoc + nbDirs[rogue.impaleDirection][0];
				y2 = player.yLoc + nbDirs[rogue.impaleDirection][1];
			}
			player.xLoc = x;
			player.yLoc = y;
			if (rogue.impaleDirection >= 0) {
				if (!i) {
					refreshDungeonCell(x2, y2);
				}
				if (bolt == BOLT_BLINKING && i + 1< numCells
					&& cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
					rogue.impaleDirection = tDirs[listOfCoordinates[i+1][0]-x+1][listOfCoordinates[i+1][1]-y+1];
				} else {
					rogue.impaleDirection = NO_DIRECTION;
				}
			}
			updateVision(true);
			backUpLighting(lights);
		}
		
		// Monsters can be impaled when blinking or pushed. Ignoring x-ray flags for the moment.
		if (((bolt == BOLT_BLINKING && shootingMonst && shootingMonst != &player && shootingMonst->creatureState != MONSTER_ALLY) || (bolt == BOLT_FORCE && pushedMonst && pushedMonst != &player))
			&& rogue.impaleDirection >= 0 && x == (player.xLoc + nbDirs[rogue.impaleDirection][0]) && y == (player.yLoc + nbDirs[rogue.impaleDirection][1])) {
			rogue.impaleDirection = NO_DIRECTION;
			refreshDungeonCell(x, y);
			if (attack(&player, bolt == BOLT_BLINKING ? shootingMonst : pushedMonst, false, true)) {
				if ((bolt == BOLT_BLINKING ? shootingMonst : pushedMonst)->bookkeepingFlags & MONST_IS_DYING) {
					rogue.impaleDirection = tDirs[x-player.xLoc+1][y-player.yLoc+1];
					if ((bolt == BOLT_BLINKING ? shootingMonst : pushedMonst)->info.bloodType
						&& dungeonFeatureCatalog[(bolt == BOLT_BLINKING ? shootingMonst : pushedMonst)->info.bloodType].tile
						&& tileCatalog[dungeonFeatureCatalog[(bolt == BOLT_BLINKING ? shootingMonst : pushedMonst)->info.bloodType].tile].foreColor) {
						rogue.weaponBloodstainColor = *tileCatalog[dungeonFeatureCatalog[(bolt == BOLT_BLINKING ? shootingMonst : pushedMonst)->info.bloodType].tile].foreColor;
					}
					if (bolt == BOLT_BLINKING) {
						shootingMonst = NULL;
					} else {
						pushedMonst = NULL;
					}
				} else {
					landMonster(x, y, bolt == BOLT_BLINKING ? shootingMonst : pushedMonst);
				}
			}
			if ((bolt == BOLT_BLINKING) || !(boltHasFlags & (BOLT_BOUNCES | BOLT_PENETRATING))) {
				break;
			}
		}
				
		// Firebolts light things on fire, and the effect is updated in realtime.
		if (!monst && bolt == BOLT_FIRE && !(boltHasFlags & BOLT_SELECTIVE)) {
			if (exposeTileToFire(x, y, true)) {
				updateVision(true);
				backUpLighting(lights);
				autoID = true;
			}
		}
		
		// Naga bolts flood intervening grids - only every third grid for performance reasons.
		if (bolt == BOLT_NAGA && !(i % 3) && !(boltHasFlags & BOLT_SELECTIVE)) {
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_NAGA_DEEP_WATER], true, false);
			autoID = true;
		}
		
		// Spider bolts web intervening grids.
		if (bolt == BOLT_SPIDER && !(boltHasFlags & BOLT_SELECTIVE)) {
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_SPIDER_WEB], true, false);
			autoID = true;
		}
		
		// Nature wands build bridges over chasms.
		if (bolt == BOLT_NATURE && (boltHasFlags & BOLT_FROM_WAND) && !(boltHasFlags & BOLT_SELECTIVE) && (cellHasTerrainFlag(x, y, T_AUTO_DESCENT))) {
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_NATURE_BRIDGE], true, false);
		}
		
		// Lightning promotes some grids
		if (bolt == BOLT_LIGHTNING && cellHasTerrainFlag2(x, y, (T2_PROMOTES_WITH_LIGHTNING)) && !(boltHasFlags & BOLT_SELECTIVE)) {
			for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
				if (tileCatalog[pmap[x][y].layers[layer]].flags2 & T2_PROMOTES_WITH_LIGHTNING) {
					promoteTile(x, y, layer, false);
					autoID = true;
				}
			}
		}
		
		// Update the visual effect of the bolt. This lighting effect is expensive; do it only if the player can see the bolt.
		if (boltInView) {
			demoteVisibility();
			restoreLighting(lights);
			for (k = min(i, boltLength + 2); k >= 0; k--) {
				if (k < initialBoltLength) {
					paintLight(&boltLights[k], listOfCoordinates[i-k][0], listOfCoordinates[i-k][1], false, false);
				}
			}
		}
		boltInView = false;
		updateFieldOfViewDisplay(false, true);
		for (k = min(i, boltLength + 2); k >= 0; k--) {
			if (rogue.playbackOmniscience || (pmap[listOfCoordinates[i-k][0]][listOfCoordinates[i-k][1]].flags & ANY_KIND_OF_VISIBLE)) {
				hiliteCell(listOfCoordinates[i-k][0], listOfCoordinates[i-k][1], boltColor, max(0, 100 - k * 100 / (boltLength)), false);
			}
            if (pmap[listOfCoordinates[i-k][0]][listOfCoordinates[i-k][1]].flags & IN_FIELD_OF_VIEW) {
                boltInView = true;
            }
		}
		if (!fastForward && (boltInView || rogue.playbackOmniscience)) {
			fastForward = rogue.playbackFastForward || pauseBrogue(10);
		}
		
		// Handle bolt reflection off of creatures (reflection off of terrain is handled further down).
		if (monst && (projectileReflects(shootingMonst, monst)
					  || (((bolt == BOLT_LIGHTNING && !(boltHasFlags & BOLT_PENETRATING)) || (bolt == BOLT_FIRE))  // penetrating lightning doesn't reflect
						  && shieldBlocks(monst, i, damage, false))) && i < DCOLS*2 && !(boltHasFlags & BOLT_SELECTIVE)
							&& (!(boltHasFlags & BOLT_X_RAY) || (boltHasFlags & BOLT_X_RAY_EXITS))) {
			if (projectileReflects(shootingMonst, monst)) { // if it scores another reflection roll, reflect at caster
				numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, !alreadyReflected);
			} else {
				numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
			}
			
			numberOfReflectsLeft--;
			alreadyReflected = true;
			
			if (boltInView) {
				monsterName(monstName, monst, true);
				sprintf(buf, "%s deflect%s the bolt", monstName, (monst == &player ? "" : "s"));
				combatMessage(buf, 0);
				
				if (monst == &player
					&& rogue.armor
					&& (rogue.armor->enchant2 == A_REFLECTION
						|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
							&& (rogue.armor->hiddenRunicEnchantsRequired <= 0) && rogue.armor->hiddenRunic == A_IMMUNITY))
					&& !(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)) {
					
					rogue.armor->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
				}

				if (monst == &player
					&& rogue.shield
					&& (rogue.shield->enchant2 == A_REFLECTION
						|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
							&& (rogue.shield->hiddenRunicEnchantsRequired <= 0) && rogue.shield->hiddenRunic == A_IMMUNITY))
					&& !(rogue.shield->flags & ITEM_RUNIC_IDENTIFIED)) {
					
					rogue.shield->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
				}
			}
			continue;
		}

		if (bolt == BOLT_FORCE && pushedMonst) {
			boltLength -= 5;
			if(boltLength <= 0) {
				break;
			}
		}
		
		if (bolt == BOLT_BLINKING || ((bolt == BOLT_BECKONING || bolt == BOLT_TELEPORT || bolt == BOLT_MIRRORED_TOTEM) && (boltHasFlags & BOLT_FROM_STAFF))
			|| ((boltHasFlags & BOLT_DISTANCE) && bolt != BOLT_BECKONING)) {
			boltLevel = (boltDistance - i) / 2 + 1;
			boltLength = boltLevel * 5 * power;
			for (j=0; j<i; j++) {
				refreshDungeonCell(listOfCoordinates[j][0], listOfCoordinates[j][1]);
			}
			if (i >= boltDistance) {
				break;
			}
		}
		
		// nature bolt passes through plants already planted. this allows runic bolts of various types to plant rows of crops
		if (bolt == BOLT_NATURE && cellHasTerrainFlag(x, y, T3_IS_PLANTED)) {
			continue;
		}
		
		// nature bolts plant something if they hit a fertile grid and are passing through three grids of the same plantable terrain
		if (bolt == BOLT_NATURE && i >= 2 && cellHasTerrainFlag3(x, y, T3_IS_FERTILE) && gridPlantingTerrain(x, y)
			&& gridPlantingTerrain(x, y) == gridPlantingTerrain(listOfCoordinates[i-1][0], listOfCoordinates[i-1][1])
			&& gridPlantingTerrain(listOfCoordinates[i-1][0], listOfCoordinates[i-1][1]) == gridPlantingTerrain(listOfCoordinates[i-2][0], listOfCoordinates[i-2][1])) {
			break;
		}
																						   
		// Force bolts pick up the monster. Note we ignore the x-ray flag, to allow force bolts to
		// knock monsters through walls/other monsters.
		if (bolt == BOLT_FORCE && monst && !(monst->info.flags & MONST_IMMOBILE) && !(monst->bookkeepingFlags & MONST_SUBMERGED) && !(boltHasFlags & BOLT_SELECTIVE)) {
			pushedMonst = monst;
			pmap[x][y].flags &= ~HAS_MONSTER;
			monst = NULL;
		}
		
		// Lightning, bouncing, fire, penetrating bolts hit targets as they travel.
		if (monst && (pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER)) && !(monst->bookkeepingFlags & MONST_SUBMERGED)
			&& (bolt == BOLT_LIGHTNING || bolt == BOLT_FIRE || boltHasFlags & (BOLT_BOUNCES | BOLT_PENETRATING)) 
			&& !(boltHasFlags & BOLT_SELECTIVE)
			&& (!(boltHasFlags & BOLT_X_RAY) || (boltHasFlags & BOLT_X_RAY_EXITS))) {

			hp = monst->currentHP;
			if (bolt != BOLT_DETONATION || !(boltHasFlags & BOLT_EXPLODES)) {
				autoID |= zapEffect(originLoc, shootingMonst, boltInView, x, y, bolt, boltLevel, power, damage, boltHasFlags);
			}
			
			// determine damage normally for each bolt while we have reflects left, we're a penetrating bolt of fire, or we're not a fire bolt and not penetrating
			if (numberOfReflectsLeft > 0 || (bolt == BOLT_FIRE && (boltHasFlags & (BOLT_PENETRATING))) || (bolt != BOLT_FIRE && !(boltHasFlags & (BOLT_PENETRATING)))) {
				damage = boltDamage(boltLevel * power);
			} else if (damage > hp) {
				damage -= hp;
			} else {
				damage = 0;
				break;
			}
			
			numberOfReflectsLeft--;
	
			switch(bolt) {
				case BOLT_OBSTRUCTION:
					feat = dungeonFeatureCatalog[DF_FORCEFIELD];
					feat.probabilityDecrement = max(1, 75 * pow(0.8, boltLevel * power));
					spawnDungeonFeature(x, y, &feat, true, false);
					autoID = true;
					break;
				case BOLT_TUNNELING:
					if (boltHasFlags & BOLT_EXPLODES) {
						crystalize(x, y, boltLevel * power * 0.5 + 1);
						autoID = true;
					}
					break;
				case BOLT_LIGHTNING:
					// ground lightning if the target hit is in water
					if (cellHasTerrainFlag2(x, y, T2_GROUND_LIGHTNING) || boltHasFlags & BOLT_EXPLODES) {
						newLoc[0] = x;
						newLoc[1] = y;
						
						spark(newLoc, boltLevel * power, damage/min(MAX_SPARKS, boltLevel));
						autoID = true;
					}			
					break;
				case BOLT_FIRE:
					if (boltHasFlags & BOLT_EXPLODES) {
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
						autoID = true;
					}
					break;
				case BOLT_POISON:
					if (boltHasFlags & BOLT_EXPLODES) {
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
						autoID = true;
					}
					break;
				case BOLT_BLINKING:
					if (boltHasFlags & BOLT_EXPLODES) {
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DARKNESS_POTION], true, false);
						autoID = true;
					}
					break;
				case BOLT_DETONATION:
					if (boltHasFlags & BOLT_EXPLODES) {
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_DETONATION], true, false);
						autoID = true;
					}
					break;
				case BOLT_FORCE:
					if (boltHasFlags & BOLT_EXPLODES) {
						hitCount = makeHitList(hitList, ATTACKS_ALL_ADJACENT, x, y, i ? tDirs[x-listOfCoordinates[i-1][0]+1][y-listOfCoordinates[i-1][1]+1] : tDirs[x-originLoc[0]+1][y-originLoc[1]+1]);
						// Attack!
						for (j=0; j<hitCount; j++) {
							if (hitList[j]) {
								newLoc[0] = hitList[j]->xLoc;
								newLoc[1] = hitList[j]->yLoc;
								zap(newLoc, originLoc, bolt, boltLevel, power, boltHasFlags & ~BOLT_EXPLODES);
							}
						}
						autoID = true;
					}
					break;					
				default:
					break;
			}
			
			boltHasFlags &= ~(BOLT_EXPLODES);
			
			// ground lightning if in water and has no other reason to keep travelling
			if (bolt == BOLT_LIGHTNING && !(boltHasFlags & (BOLT_BOUNCES | BOLT_PENETRATING)) && cellHasTerrainFlag2(x, y, T2_GROUND_LIGHTNING)) {
				break;
			}
		}
		
		// Penetrating, lunging, impaling blinks hit targets ahead of them. Impaling also does, except for monsters in walls.
		if (bolt == BOLT_BLINKING && ((boltHasFlags & BOLT_PENETRATING) || (shootingMonst == &player && rogue.weapon && (rogue.weapon->flags & (ITEM_LUNGE_ATTACKS | ITEM_IMPALES)))) 
				&& i + 1 < numCells
						&& (pmap[listOfCoordinates[i+1][0]][listOfCoordinates[i+1][1]].flags & (HAS_PLAYER | HAS_MONSTER)) 
			&& !(monsterAtLoc(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1])->bookkeepingFlags & MONST_SUBMERGED)) {

			hitCount = makeHitList(hitList, rogue.weapon->flags, x, y, tDirs[listOfCoordinates[i+1][0]-x+1][listOfCoordinates[i+1][1]-y+1]);
			
			// Attack!
			for (j=0; j<hitCount; j++) {
				if (hitList[j]
					&& monstersAreEnemies(&player, hitList[j])
					&& !(hitList[j]->bookkeepingFlags & MONST_IS_DYING)) {
					attack(shootingMonst, hitList[j], true, false); // Always lunge, regardless of weapon if penetrating
				}
			}
			if (!(boltHasFlags & BOLT_PENETRATING)) {
				break;
			}
		}
		
		// Some bolts halt at the square before they hit something,
		// or the square after they hit something if an x-ray bolt.
		if ((bolt == BOLT_BLINKING || bolt == BOLT_OBSTRUCTION || (bolt > BOLT_SENTRY && bolt != BOLT_UNDERWORM)
			 || (boltHasFlags & (BOLT_EXPLODES | BOLT_CHAINING)) || (bolt == BOLT_FORCE && pushedMonst))
			&& ((boltHasFlags & BOLT_X_RAY_EXITS)
				|| (i + 1 < numCells
					&& (cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1],
										   (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
						|| ((pmap[listOfCoordinates[i+1][0]][listOfCoordinates[i+1][1]].flags & (HAS_PLAYER | HAS_MONSTER)) 
							&& !(monsterAtLoc(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1])->bookkeepingFlags & MONST_SUBMERGED)
							&& !(boltHasFlags & (BOLT_SELECTIVE | BOLT_CHAINING))))
					&& (!(boltHasFlags & BOLT_X_RAY)
						|| (listOfCoordinates[i+1][0] == 0 && listOfCoordinates[i+1][1] == 0 && listOfCoordinates[i+1][0] == DCOLS - 1 && listOfCoordinates[i+1][1] == DROWS - 1)
						|| i + 2 >= numCells // x-ray bolts explode early if the square after the exit isn't empty
						|| cellHasTerrainFlag(listOfCoordinates[i+2][0], listOfCoordinates[i+2][1],
											  (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
						|| (bolt != BOLT_BLINKING && bolt < BOLT_SENTRY)
						|| (pmap[listOfCoordinates[i+2][0]][listOfCoordinates[i+2][1]].flags & (HAS_PLAYER | HAS_MONSTER)))))) {
			if (pushedMonst) {
				if (i + 1 < numCells && cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1],
											  (T_OBSTRUCTS_VISION)) && cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1],
																						  (T_OBSTRUCTS_PASSABILITY))) {
					pushDamage = damage * 3; // triple damage if forced monster hits a wall
				} else {
					pushDamage = 0;
				}
				hp = pushedMonst->currentHP;
				landMonster(x, y, pushedMonst);				
				pushedMonst = NULL;
				autoID |= zapEffect(originLoc, shootingMonst, boltInView, x, y, bolt, boltLevel, power, pushDamage ? pushDamage : damage, boltHasFlags);
				
				// determine damage normally for each bolt while we have reflects left, we're a penetrating bolt of fire, or we're not a fire bolt and not penetrating
				if (numberOfReflectsLeft > 0) {
					damage = boltDamage(boltLevel * power);
				} else if (pushDamage && (boltHasFlags & (BOLT_PENETRATING))) {
					if (pushDamage >= hp) {
						damage -= (hp + 2)/3;
					}
					if (damage == 0) {
						break;
					}
				} else if (damage > hp && (boltHasFlags & (BOLT_PENETRATING))) {
					damage -= hp;
				} else {
					damage = 0;
					break;
				}
			} else {
				break;
			}
		}
		
        // Stop when we hit something -- a wall or a non-submerged creature or target grid for precision bolts
		// and mirrored totems.
        // However, lightning, tunneling, underworm and sentry don't stop when they hit creatures,
		// and tunneling and underworm continues through walls, and sentries and force through foliage,
		// doors and (for sentry) wooden barriers.
		if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
			|| (((pmap[x][y].flags & HAS_PLAYER) || ((pmap[x][y].flags & HAS_MONSTER) && monst && !(monst->bookkeepingFlags & MONST_SUBMERGED)))
			&& bolt != BOLT_LIGHTNING && bolt != BOLT_SENTRY && bolt != BOLT_NATURE && !(boltHasFlags & BOLT_BOUNCES))
			|| (((boltHasFlags & BOLT_PRECISION) || bolt == BOLT_MIRRORED_TOTEM) && targetLoc[0] == x && targetLoc[1] == y)) {
			
			// Force pushes through passable terrain which can be altered
			if (bolt == BOLT_FORCE && cellHasTerrainFlag(x, y, (T_PROMOTES_ON_STEP)) && cellHasTerrainFlag(x, y, (T_VANISHES_UPON_PROMOTION))) {
				if (!(bolt & BOLT_SELECTIVE)) {
					for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
						if (tileCatalog[pmap[x][y].layers[layer]].flags & T_PROMOTES_ON_STEP) {
							promoteTile(x, y, layer, false);
						}
					}
				}
			}
			// Sentry tunnels through passable but vision blocking or flammable terrain
			else if (bolt == BOLT_SENTRY && (!cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY)) || cellHasTerrainFlag(x, y, (T_IS_FLAMMABLE)))) {
				if (!(bolt & BOLT_SELECTIVE)) {
					tunnelize(x, y);
				}
			}
			// Tunnelling, underworm, x-ray through wall/monster
			else if ((bolt == BOLT_TUNNELING || bolt == BOLT_UNDERWORM || ((boltHasFlags & BOLT_X_RAY) && !(boltHasFlags & BOLT_X_RAY_EXITS)))
				&& x > 0 && y > 0 && x < DCOLS - 1 && y < DROWS - 1) { // don't tunnelize the outermost walls

				if ((boltHasFlags & BOLT_X_RAY) && !(boltHasFlags & BOLT_X_RAY_EXITS)) {
					boltHasFlags |= BOLT_X_RAY_EXITS;
				} else if (boltHasFlags & BOLT_SELECTIVE) {
					// do nothing
				} else if (!cellHasTerrainFlag2(x, y, (T2_TUNNELIZE_IGNORES_GRID))) {
					if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY))) {
						if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_VISION)) && bolt == BOLT_UNDERWORM) {
							x2 = x;
							y2 = y;
						}
						tunnelizeOrBurrow(x, y, bolt);
						if (i > 0 && x != listOfCoordinates[i-1][0] && y != listOfCoordinates[i-1][1]) {
							if (rand_percent(50)) {
								tunnelizeOrBurrow(listOfCoordinates[i-1][0], y, bolt);
							} else {
								tunnelizeOrBurrow(x, listOfCoordinates[i-1][1], bolt);
							}
						} else if (i == 0 && x > 0 && y > 0 && x < DCOLS - 1 && y < DROWS - 1) {
							if (rand_percent(50)) {
								tunnelizeOrBurrow(originLoc[0], y, bolt);
							} else {
								tunnelizeOrBurrow(x, originLoc[1], bolt);
							}
						}
					}
					updateVision(true);
					backUpLighting(lights);
					autoID = true;
					boltLength -= 5;
					for (j=0; j<i; j++) {
						refreshDungeonCell(listOfCoordinates[j][0], listOfCoordinates[j][1]);
					}
					if(boltLength <= 0) {
						refreshDungeonCell(listOfCoordinates[i-1][0], listOfCoordinates[i-1][1]);
						refreshDungeonCell(x, y);
						break;
					}
				}
			// We've reached an outer wall - find space for the underworm
			} else if (bolt == BOLT_UNDERWORM) {
				if (x2 == -1 || y2 == -1) {
					return false;
				}
				x = x2;
				y = y2;
				break;
				// Lightning through conductive terrain
			} else if (bolt == BOLT_LIGHTNING && cellHasTerrainFlag2(x, y, (T2_CONDUCTIVE_WALL))) {
				// continue through conductive terrain
			// We're trying to summon a monster selectively but haven't found space before reaching a wall
			} else if ((boltHasFlags & BOLT_SELECTIVE) && bolt > BOLT_SENTRY) {
				return false;
			} else {
				break;
			}
		}
		
		// does the bolt bounce off the wall?
		// Can happen with when shooting a tunneling bolt into an impregnable wall, or with a lightning bolt which has not struck twice,
		// or a sentry bolt into a terrain which prevents egress but which is not flammable, or reflective terrain.
		
		if (i + 1 < numCells
			&& (!(boltHasFlags & BOLT_X_RAY) || (boltHasFlags & BOLT_X_RAY_EXITS))
			&& (cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1],
								  (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY)))
			&& (projectileReflects(shootingMonst, NULL)
				|| (bolt == BOLT_SENTRY && !(cellHasTerrainFlag(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1], (T_OBSTRUCTS_VISION)))
					&& !(cellHasTerrainType(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1], (T_IS_FLAMMABLE))))
				|| (numberOfReflectsLeft > 0 && (bolt != BOLT_LIGHTNING || !(boltHasFlags & BOLT_PENETRATING)) // penetrating lightning bolts don't bounce off walls
					&& !monsterAtLoc(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1])
					&& (bolt != BOLT_LIGHTNING || !cellHasTerrainFlag2(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1], (T2_CONDUCTIVE_WALL)))) // only lightning is conducted
				|| ((bolt == BOLT_TUNNELING || bolt == BOLT_UNDERWORM) && (pmap[listOfCoordinates[i+1][0]][listOfCoordinates[i+1][1]].flags & IMPREGNABLE))
				|| (cellHasTerrainFlag2(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1],
										(T2_REFLECTS_BOLTS))))
			&& i < DCOLS*2) {
			
			sprintf(buf, "the bolt reflects off of %s", tileText(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1]));
			
			numberOfReflectsLeft--;
			
			if (projectileReflects(shootingMonst, NULL)) { // if it scores another reflection roll, reflect at caster
				numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, !alreadyReflected);
			} else {
				numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
			}
			
			alreadyReflected = true;
			
			if (boltInView) {
				combatMessage(buf, 0);
			}
			continue;
		}

		// Handle x-ray bolts which don't stop when they hit a monster.
		if (monst && (pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER)) && !(monst->bookkeepingFlags & MONST_SUBMERGED)
			&& !(boltHasFlags & BOLT_SELECTIVE)
			&& (!(boltHasFlags & BOLT_X_RAY) || (boltHasFlags & BOLT_X_RAY_EXITS))) {
			boltHasFlags &= ~BOLT_X_RAY; // Explode at the next target we hit, rather than immediately following this target
		}
	}

	/* Catcher in the outfield */
	if (bolt == BOLT_FORCE && (i == numCells || !boltLevel)) {
		damage = 0;
	}
	
	/* Find space for an underworm */
	if (bolt == BOLT_UNDERWORM) {
		if (x2 == -1 || y2 == -1) {
			return false;
		}
		x = x2;
		y = y2;
	}
	
	if (bolt == BOLT_BLINKING && shootingMonst) {
		landMonster(x, y, shootingMonst);
		autoID = true;
	} else if (bolt == BOLT_FORCE && pushedMonst) {
		landMonster(x, y, pushedMonst);
		autoID = true;
	} else if (bolt == BOLT_BECKONING) {
		if (monst && !(monst->info.flags & MONST_INANIMATE)
			&& distanceDiagonal(originLoc[0], originLoc[1], monst->xLoc, monst->yLoc) > 1) {
			fastForward = true;
		}
	}
	
	// don't apply if we've already taken effect in this grid
	if (bolt != BOLT_LIGHTNING && !(boltHasFlags & BOLT_BOUNCES) && (damage > 0 || !(boltHasFlags & BOLT_PENETRATING) || bolt == BOLT_FIRE)
		&& (damage > 0 || (boltHasFlags & BOLT_PENETRATING) || bolt != BOLT_FIRE)) {
		autoID |= (zapEffect(originLoc, shootingMonst, boltInView, x, y, bolt, boltLevel, power, damage, boltHasFlags));
		
		switch(bolt) {
			case BOLT_OBSTRUCTION:
				feat = dungeonFeatureCatalog[DF_FORCEFIELD];
				feat.probabilityDecrement = max(1, 75 * pow(0.8, boltLevel * power));
				spawnDungeonFeature(x, y, &feat, true, false);
				autoID = true;
				break;
			case BOLT_TUNNELING:
				if (boltHasFlags & BOLT_EXPLODES) {
					crystalize(x, y, boltLevel * power * 0.5 + 1);
					autoID = true;
				}			
				break;
			case BOLT_LIGHTNING:
				// ground lightning if the target hit is in water
				if (cellHasTerrainFlag2(x, y, T2_GROUND_LIGHTNING) || (boltHasFlags & BOLT_EXPLODES)) {
					newLoc[0] = x;
					newLoc[1] = y;
					
					spark(newLoc, boltLevel * power, damage/min(MAX_SPARKS, boltLevel));
					autoID = true;
				}			
				break;
			case BOLT_FIRE:
				if (boltHasFlags & BOLT_EXPLODES) {
					spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
					autoID = true;
				}
				break;
			case BOLT_POISON:
				if (boltHasFlags & BOLT_EXPLODES) {
					spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
					autoID = true;
				}
				break;
			case BOLT_BLINKING:
				if (boltHasFlags & BOLT_EXPLODES) {
					spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DARKNESS_POTION], true, false);
				}
			case BOLT_DETONATION:
				if (boltHasFlags & BOLT_EXPLODES) {
					spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_DETONATION], true, false);
					autoID = true;
				}
				break;
			case BOLT_FORCE:
				if (boltHasFlags & BOLT_EXPLODES) {
					hitCount = makeHitList(hitList, ATTACKS_ALL_ADJACENT, x, y, i ? tDirs[x-listOfCoordinates[i-1][0]+1][y-listOfCoordinates[i-1][1]+1] : tDirs[x-originLoc[0]+1][y-originLoc[1]+1]);
					// Attack!
					for (j=0; j<hitCount; j++) {
						if (hitList[j]) {
							newLoc[0] = hitList[j]->xLoc;
							newLoc[1] = hitList[j]->yLoc;
							zap(newLoc, originLoc, bolt, boltLevel, power, boltHasFlags & ~BOLT_EXPLODES);
						}
					}
					autoID = true;
				}
				break;					
			default:
				break;
		}
	}
	
	if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
		monst = monsterAtLoc(x, y);
		monsterName(monstName, monst, true);
	} else {
		monst = NULL;
	}
	
	// handle final location
	switch(bolt) {
		case BOLT_BLINKING:
			if (shootingMonst == &player) {
				// handled above for visual effect (i.e. before contrail fades)
				// increase scent turn number so monsters don't sniff around at the old cell like idiots
				rogue.scentTurnNumber += 30;
				// get any items at the destination location
				if (pmap[player.xLoc][player.yLoc].flags & HAS_ITEM) {
					pickUpItemAt(player.xLoc, player.yLoc);
				}
			}
			break;
		case BOLT_TUNNELING:
			if (autoID) {
				setUpWaypoints(); // recompute based on the new situation
			}
			break;
			
		case BOLT_NATURE:
			// prevent spawning dead man's ear/crimson caps too close together
			if (boltHasFlags & BOLT_FROM_STAFF) {
				autoID = false;
				for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
					if (monst->info.monsterID == MK_DEAD_MANS_EAR || monst->info.monsterID == MK_CRIMSON_CAP || monst->info.monsterID == MK_MANDRAKE) {
						if (distanceDiagonal(x, y, monst->xLoc, monst->yLoc) <= staffNatureMinimumGap(boltLevel * power)) {
							autoID = true;
							if (!monst->status[STATUS_MARKED]) {
								monst->status[STATUS_MARKED] = 1;
								rogue.markedMonsters++;
							}
						}
					}
				}
				if (autoID) {
					updateTelepathy();
					updateTelepathy();					
				}
				if (magicMapping(x, y, staffNatureMinimumGap(boltLevel * power) + 1, 0, 0, 0, T3_IS_PLANTED)) {
					lightFlash(&boltColor[BOLT_NATURE], 0, MAGIC_MAPPED, 15, DCOLS, player.xLoc, player.yLoc);
					autoID = true;
				}
				if (autoID) {
					message("there are magical plants too close to grow another.", false);
					break;
				}
			}
			
			if (gridPlantingTerrain(x,y)) {
				autoID = true;
				switch (gridPlantingTerrain(x,y)) {
					case DEEP_WATER:
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BOLT_NATURE_WATER], true, false);
						break;
					case GRASS:
						if (spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BLOODFLOWER_PODS_GROW_INITIAL], true, true)) {
							pmap[x][y].layers[SURFACE] = BLOODFLOWER_STALK;
						} else {
							pmap[x][y].layers[SURFACE] = BLOODFLOWER_STALK_DYING;
						}
						break;
					case DEAD_GRASS:
						if (spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_STINKFRUIT_PODS_GROW_INITIAL], true, true)) {
							pmap[x][y].layers[SURFACE] = STINKFRUIT_STALK;
						} else {
							pmap[x][y].layers[SURFACE] = STINKFRUIT_STALK_DYING;
						}
						break;
					case GRAY_FUNGUS:
						bolt = BOLT_DEAD_MANS_EAR;
						break;
					case LUMINESCENT_FUNGUS:
						bolt = BOLT_CRIMSON_CAP;
						break;
					case LICHEN:
						spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_CREEPER_FUNGUS_REGROW], true, false);					
						break;
					case HAY:
						pmap[x][y].layers[SURFACE] = WITCH_HAZEL_FLOWER;
						break;
					case RED_BLOOD:
						bolt = BOLT_MANDRAKE;
						break;
				}
			}
			if (bolt == BOLT_NATURE) {
				break;
			}
			// Fall through
		case BOLT_MIRRORED_TOTEM:
			if (((pmap[x][y].flags & IS_IN_MACHINE) && magicMapping(x, y, 0, pmap[x][y].machineNumber, (T_IS_WIRED), (T2_CIRCUIT_BREAKER), 0))
				|| (x > 0 && (pmap[x-1][y].flags & IS_IN_MACHINE) && magicMapping(x-1, y, 0, pmap[x-1][y].machineNumber, (T_IS_WIRED), (T2_CIRCUIT_BREAKER), 0))
				|| (y > 0 && (pmap[x][y-1].flags & IS_IN_MACHINE) && magicMapping(x, y-1, 0, pmap[x][y-1].machineNumber, (T_IS_WIRED), (T2_CIRCUIT_BREAKER), 0))
				|| (x < DCOLS && (pmap[x+1][y].flags & IS_IN_MACHINE) && magicMapping(x+1, y, 0, pmap[x+1][y].machineNumber, (T_IS_WIRED), (T2_CIRCUIT_BREAKER), 0))
				|| (y < DROWS && (pmap[x][y+1].flags & IS_IN_MACHINE) && magicMapping(x, y+1, 0, pmap[x][y+1].machineNumber, (T_IS_WIRED), (T2_CIRCUIT_BREAKER), 0))) {
				message("there are magical machines too close to place another.", false);
				lightFlash(boltColors[BOLT_MIRRORED_TOTEM], 0, MAGIC_MAPPED, 15, DCOLS, player.xLoc, player.yLoc);
				break;
			}
			// Fall through
		case BOLT_SENTRY:
		case BOLT_ZOMBIE:
			//		case BOLT_PHANTOM:
		case BOLT_BLOAT:
		case BOLT_SPIDER:
		case BOLT_NAGA:
		case BOLT_PIXIE:
		case BOLT_TOAD:
		case BOLT_UNDERWORM:
		case BOLT_MANDRAKE:
		case BOLT_DEAD_MANS_EAR:
		case BOLT_CRIMSON_CAP:
			
			// Sentries and underworms must spawn on walls. Note we are more liberal with sentry placement than when spawning sentries normally.
			if (!(pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) && ((bolt != BOLT_SENTRY && bolt != BOLT_UNDERWORM) ||
													  (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY)) && cellHasTerrainFlag(x, y, (T_OBSTRUCTS_VISION))))) {
				
				// We could change horde to 0 to always spawn a monster for the correct depth, but instead just give an equal chance of each type
				// This ensures that the sentry bolts are sufficiently risky to use at all depths
				
				k = 0; // Count of horde found
				
				for (j = 0; j < NUMBER_HORDES; j++)
				{
					if ((hordeCatalog[j].flags & (HORDE_SENTRY << (bolt - BOLT_SENTRY))) && (rand_range(1, ++k) == 1)) horde = j;
				}
				
				if (bolt == BOLT_TOAD) {
					k = rand_range(1, max(2, boltLevel));
				} else {
					k = 1;
				}

				for (j = 0; j < k; j++) {
					getQualifyingLocNear(newLoc, x, y, true, 0, (bolt != BOLT_SENTRY && bolt != BOLT_UNDERWORM ? T_OBSTRUCTS_PASSABILITY : 0), (HAS_MONSTER | HAS_PLAYER), false, false);
					monst = spawnHorde(horde, newLoc[0], newLoc[1], 0, (HORDE_SENTRY << (bolt - BOLT_SENTRY)));
					if (monst) {
						// prevent abuse
						if (staffTable[bolt].identified) {
							monst->info.flags |= (MONST_NO_EXPERIENCE);
						}
						
						if (!(monst->bookkeepingFlags & MONST_IS_DORMANT)) {
							// refreshDungeonCell(x, y);
							// monst->creatureState = MONSTER_TRACKING_SCENT;
							// monst->ticksUntilTurn = player.movementSpeed;
							wakeUp(monst);
							//fadeInMonster(monst);
						}
						autoID = true;
						
						switch (bolt) {
							case BOLT_SENTRY:
								duration = boltSentryDuration(boltLevel);
								monst->protectionAmount = max(monst->protectionAmount, boltSentryProtection(boltLevel * power));
								monst->status[STATUS_SHIELDED] = min(20, monst->protectionAmount);
								monst->maxStatus[STATUS_SHIELDED] = monst->status[STATUS_SHIELDED];
								break;
							case BOLT_ZOMBIE:
								duration = boltZombieDuration(boltLevel);
								if ((boltHasFlags & BOLT_FROM_STAFF) && rand_range(1,100) <= staffZombieAflame(boltLevel)) {
									exposeCreatureToFire(monst);
								}
								break;
								//					case BOLT_PHANTOM:
								//						duration = boltPhantomDuration(boltLevel);
								//						if (rand_range(1,100) <= boltPhantomDarkness(boltLevel)) {
								//							spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_STAFF_PHANTOM_DARKNESS], true, false);
								//						}
								//						break;
							case BOLT_BLOAT:
								if (boltHasFlags & BOLT_FROM_WAND) {
									duration = wandBloatDuration(boltLevel);
								} else {
									duration = 0;
								}
								break;
							case BOLT_SPIDER:
								duration = boltSpiderDuration(boltLevel);
								break;
							case BOLT_NAGA:
								duration = boltNagaDuration(boltLevel);
								break;
							case BOLT_PIXIE:
								duration = boltPixieDuration(boltLevel);
								break;
							case BOLT_TOAD:
								duration = boltToadDuration(boltLevel);
								break;
							case BOLT_UNDERWORM:
								duration = boltUnderwormDuration(originalBoltLevel); // doesn't take effect enough otherwise
								break;
							case BOLT_MANDRAKE:
								spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_MANDRAKE_PLANTED], true, false);
								duration = boltMandrakeDuration(boltLevel);
								break;
							case BOLT_DEAD_MANS_EAR:
								duration = boltDeadMansEarDuration(boltLevel);
								break;
							case BOLT_CRIMSON_CAP:
								duration = boltCrimsonCapDuration(boltLevel);
								break;
							case BOLT_MIRRORED_TOTEM:
								pmap[x][y].machineNumber = 100; // should never have 100 machines on a level
								pmap[x][y].flags |= IS_IN_MACHINE;
								if (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION) { // paranoia
									monst->machineHome = 100;
								}
								if (boltHasFlags & BOLT_PRECISION) {
									duration = boltMirroredTotemDuration(boltLevel);
									pmap[x][y].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
									pmap[x][y].flags |= IS_IN_MACHINE;
									if ((boltHasFlags & BOLT_X_RAY_EXITS) && i + 1 < numCells) {
										pmap[listOfCoordinates[i+1][0]][listOfCoordinates[i+1][1]].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
										pmap[listOfCoordinates[i+1][0]][listOfCoordinates[i+1][1]].flags |= IS_IN_MACHINE;								
									} else if (i > 0) {
										pmap[listOfCoordinates[i-1][0]][listOfCoordinates[i-1][1]].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
										pmap[listOfCoordinates[i-1][0]][listOfCoordinates[i-1][1]].flags |= IS_IN_MACHINE;								
									} else {
										pmap[originLoc[0]][originLoc[1]].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
										pmap[originLoc[0]][originLoc[1]].flags |= IS_IN_MACHINE;				
									}
									if (boltHasFlags & BOLT_DISTANCE) {
										if (i > 0) {
											pmap[listOfCoordinates[0][0]][listOfCoordinates[0][1]].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
											pmap[listOfCoordinates[0][0]][listOfCoordinates[0][1]].flags |= IS_IN_MACHINE;								
										}
										//									pmap[originLoc[0]][originLoc[1]].layers[SURFACE] = MACHINE_GLYPH_INACTIVE;
										//									pmap[originLoc[0]][originLoc[1]].flags |= IS_IN_MACHINE;									
									}
								} else {
									if (boltHasFlags & BOLT_FROM_STAFF) {
										duration = boltMirroredTotemDuration(boltLevel);
									} else {
										duration = 1000;
									}
									duration = boltMirroredTotemDuration(boltLevel);
									if (!spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_GLYPH_CIRCLE_SMALL], true, !(boltHasFlags & BOLT_FROM_STAFF) ? true : false)) { // limit duration if blocking and a wand
										duration = boltMirroredTotemDuration(boltLevel);
										spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_GLYPH_CIRCLE_SMALL], true, false);
									}
									pmap[x-1][y].machineNumber = 100;
									pmap[x][y-1].machineNumber = 100;
									pmap[x+1][y].machineNumber = 100;
									pmap[x][y+1].machineNumber = 100;
									pmap[x-1][y].flags |= IS_IN_MACHINE;
									pmap[x][y-1].flags |= IS_IN_MACHINE;
									pmap[x+1][y].flags |= IS_IN_MACHINE;
									pmap[x][y+1].flags |= IS_IN_MACHINE;
									if (boltHasFlags & BOLT_DISTANCE) {
										spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_GLYPH_CIRCLE_SMALL], true, false);
										pmap[player.xLoc-1][player.yLoc].machineNumber = 100;
										pmap[player.xLoc][player.yLoc-1].machineNumber = 100;
										pmap[player.xLoc+1][player.yLoc].machineNumber = 100;
										pmap[player.xLoc][player.yLoc+1].machineNumber = 100;
										pmap[player.xLoc-1][player.yLoc].flags |= IS_IN_MACHINE;
										pmap[player.xLoc][player.yLoc-1].flags |= IS_IN_MACHINE;
										pmap[player.xLoc+1][player.yLoc].flags |= IS_IN_MACHINE;
										pmap[player.xLoc][player.yLoc+1].flags |= IS_IN_MACHINE;
									}
								}
								break;
							default:
								duration = 0;
								break;
						}
						
						// make mandrake bolts different enough from pixie bolts; immobile allies from blocking hallways
						if (bolt == BOLT_MANDRAKE || (bolt != BOLT_SENTRY && (monst->info.flags & MONST_IMMOBILE))) {
							duration *= power;
							monst->status[STATUS_LIFESPAN_REMAINING] = monst->maxStatus[STATUS_LIFESPAN_REMAINING] = duration;
						} else {
							if (boltHasFlags & BOLT_FROM_WAND) {
								duration /= power;
							} else {
								duration *= power;
							}
							// allies become discordant with wands as a method of balancing them; enemies with staffs in order to make them useful
							monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = max(duration, monst->status[STATUS_DISCORDANT]);
						}
						
						if ((boltHasFlags & BOLT_FROM_WAND) && bolt != BOLT_BLOAT) {
							becomeAllyWith(monst);
							// allied monsters summoned by wands never gain experience
							monst->spawnDepth = 101;
						}
						
						if (!(monst->bookkeepingFlags & MONST_IS_DORMANT)) {
							refreshSideBar(-1, -1, false);
							refreshDungeonCell(monst->xLoc, monst->yLoc);
							// flashMonster(monst, boltColors[bolt], 100);
							autoID = true;
						}
					}
				}
			}
			break;			
		default:
			break;
	}
	
	// Pass back updated target
	if (boltHasFlags & BOLT_CHAINING)
	{
		targetLoc[0] = x;
		targetLoc[1] = y;
	}
	
	updateLighting();
	backUpLighting(lights);
	boltInView = true;
	refreshSideBar(-1, -1, false);
	if (boltLength > 0) {
		// j is where the front tip of the bolt would be if it hadn't collided at i
		for (j=i; j < i + boltLength + 2; j++) { // j can imply a bolt tip position that is off the map
			
			// dynamic lighting
			if (boltInView) {
				demoteVisibility();
				restoreLighting(lights);
                
                // k = j-i;
                // boltLights[k].lightRadius.lowerBound *= 2;
                // boltLights[k].lightRadius.upperBound *= 2;
                // boltLights[k].lightColor = &boltImpactColor;
                
				for (k = min(j, boltLength + 2); k >= j-i; k--) {
					if (k < initialBoltLength) {
						paintLight(&boltLights[k], listOfCoordinates[j-k][0], listOfCoordinates[j-k][1], false, false);
					}
				}
				updateFieldOfViewDisplay(false, true);
			}
			
			boltInView = false;
			
			// beam graphic
			// k iterates from the tail tip of the visible portion of the bolt to the head
			for (k = min(j, boltLength + 2); k >= j-i; k--) {
				if (pmap[listOfCoordinates[j-k][0]][listOfCoordinates[j-k][1]].flags & ANY_KIND_OF_VISIBLE) {
					hiliteCell(listOfCoordinates[j-k][0], listOfCoordinates[j-k][1], boltColor, max(0, 100 - k * 100 / (boltLength)), false);
					if (pmap[listOfCoordinates[j-k][0]][listOfCoordinates[j-k][1]].flags & ANY_KIND_OF_VISIBLE) {
						boltInView = true;
					}
				}
			}
			
			if (!fastForward && boltInView) {
				fastForward = rogue.playbackFastForward || pauseBrogue(10);
			}
		}
	}
	
	return autoID;
}

boolean zapDamage(creature *shootingMonst, creature *monst, boolean boltInView, color *zapColor, char *zapName, long damage)
{
	char buf[COLS], monstName[COLS];
	
	monsterName(monstName, monst, true);
	
	if (inflictDamage( monst, damage, zapColor)) {
		// killed monster
		if (player.currentHP <= 0) {
			if (shootingMonst == &player) {
				sprintf(buf,"Killed by a reflected %s", zapName);
				gameOver(buf, true);
			}
			return false;
		}
		if (boltInView) {
			sprintf(buf, "%s %s %s %s",
					canSeeMonster(shootingMonst) ? "the" : "a",
					zapName,
					((monst->info.flags & MONST_INANIMATE) ? "destroys" : "kills"),
					monstName);
			combatMessage(buf, messageColorFromVictim(monst));
		} else {
			sprintf(buf, "you hear %s %s", monstName, ((monst->info.flags & MONST_INANIMATE) ? "be destroyed" : "die"));
			combatMessage(buf, messageColorFromVictim(monst));
		}
		return false;
	} else {
		// monster lives
		if (monst->creatureMode != MODE_PERM_FLEEING
			&& monst->creatureState != MONSTER_ALLY
			&& (monst->creatureState != MONSTER_FLEEING || monst->status[STATUS_MAGICAL_FEAR])) {
			monst->creatureState = MONSTER_TRACKING_SCENT;
			monst->status[STATUS_MAGICAL_FEAR] = 0;
		}
		if (boltInView) {
			sprintf(buf, "%s %s hits %s",
					canSeeMonster(shootingMonst) ? "the" : "a",
					zapName,
					monstName);
			combatMessage(buf, messageColorFromVictim(monst));
		}
		
		moralAttack(shootingMonst, monst);
	}
	
	return true;
}

boolean zapEffect(short originLoc[2], creature *shootingMonst, boolean boltInView, short x, short y, enum boltType bolt, short boltLevel, float power, long damage, unsigned long boltHasFlags) {
	short i, j, newLoc[2];
	short poisonDamage;
	creature *monst = NULL, *newMonst;
	char buf[COLS], monstName[COLS];
	boolean autoID = false;
	extern color forceFieldColor;
	
	if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
		monst = monsterAtLoc(x, y);
		if (monst && monst->bookkeepingFlags & MONST_SUBMERGED) {
			monst = NULL;
		} else {
			monsterName(monstName, monst, true);
		}
	} else {
		monst = NULL;
	}

	switch(bolt) {
		case BOLT_TELEPORT:
			if (monst && !(monst->info.flags & MONST_INANIMATE)) {
				if (monst->bookkeepingFlags & MONST_CAPTIVE) {
					freeCaptive(monst);
				}
				teleport(monst);
				refreshSideBar(-1, -1, false);
			}
			break;
		case BOLT_BECKONING:
			if (monst && !(monst->info.flags & MONST_INANIMATE)
				&& distanceDiagonal(originLoc[0], originLoc[1], monst->xLoc, monst->yLoc) > 1) {
				if (canSeeMonster(monst)) {
					autoID = true;
				}
				if (monst->bookkeepingFlags & MONST_CAPTIVE) {
					freeCaptive(monst);
				}
				newLoc[0] = monst->xLoc;
				newLoc[1] = monst->yLoc;
				zap(newLoc, originLoc, BOLT_BLINKING, max(1, (distanceDiagonal(originLoc[0], originLoc[1], newLoc[0], newLoc[1]) - 2) / 2), power, boltHasFlags);
				if (monst->ticksUntilTurn < player.attackSpeed+1) {
					monst->ticksUntilTurn = player.attackSpeed+1;
				}
				if (canSeeMonster(monst)) {
					autoID = true;
				}
			}
			break;
		case BOLT_SLOW:
			if (monst) {
				slow(monst, boltLevel * power);
				refreshSideBar(-1, -1, false);
				flashMonster(monst, boltColors[BOLT_SLOW], 100);
				autoID = true;
			}
			break;
		case BOLT_HASTE:
			if (monst) {
				haste(monst, boltHasteDuration(boltLevel * power), false);
				refreshSideBar(-1, -1, false);
				flashMonster(monst, boltColors[BOLT_HASTE], 100);
				autoID = true;
			}
			break;
		case BOLT_POLYMORPH:
			if (monst && monst != &player && !(monst->info.flags & MONST_INANIMATE)) {
				polymorph(monst, -1, (boltHasFlags & BOLT_FROM_STAFF) ? staffPolymorphDuration(boltLevel * power) : 0);
				refreshSideBar(-1, -1, false);
				if (!monst->status[STATUS_INVISIBLE]) {
					autoID = true;
				}
			}
			break;
		case BOLT_INVISIBILITY:
            autoID = imbueInvisibility(monst, boltInvisibilityDuration(boltLevel * power), (boltHasFlags & BOLT_HIDE_DETAILS) != 0);
			break;
		case BOLT_DOMINATION:
			if (monst && monst != &player && !(monst->info.flags & MONST_INANIMATE)) {
				if (((boltHasFlags & BOLT_FROM_STAFF) && monst->creatureState != MONSTER_TRACKING_SCENT && monst->creatureState != MONSTER_FLEEING) ||
					rand_percent(boltDominate(monst))) {
					// domination succeeded
					monst->status[STATUS_DISCORDANT] = 0;
					if ((boltHasFlags & BOLT_FROM_STAFF) && monst->creatureState != MONSTER_ALLY) {
						monst->status[STATUS_DOMINATED] += staffDominationDuration(boltLevel * power);
						monst->maxStatus[STATUS_DOMINATED] = monst->status[STATUS_DOMINATED];
					}
					becomeAllyWith(monst);
					refreshSideBar(-1, -1, false);
					refreshDungeonCell(monst->xLoc, monst->yLoc);
					// prevent monster gaining experience before it gets in depth
					for (i = 0; i < NUMBER_HORDES; i++) {
						if (hordeCatalog[i].leaderType == monst->info.monsterID) {
							if (monst->spawnDepth < hordeCatalog[i].minLevel) {
								monst->spawnDepth = hordeCatalog[i].minLevel;
							}
							break;
						}
					}
					if (canSeeMonster(monst)) {
						autoID = true;
						sprintf(buf, "%s is bound to your will!", monstName);
						message(buf, false);
						flashMonster(monst, boltColors[BOLT_DOMINATION], 100);
					}
				} else if (canSeeMonster(monst)) {
					autoID = true;
					sprintf(buf, "%s resists the bolt of domination.", monstName);
					message(buf, false);
				}
			}
			break;
		case BOLT_NEGATION:
			if (monst) { // works on inanimates
				negate(monst, (boltHasFlags & BOLT_FROM_STAFF) ? staffNegationDuration(boltLevel * power): 0);
				refreshSideBar(-1, -1, false);
				flashMonster(monst, boltColors[BOLT_NEGATION], 100);
				autoID = true;
			}
			break;
		case BOLT_LIGHTNING:
			if (monst && !(boltHasFlags & BOLT_NO_DAMAGE)) {
				if (canSeeMonster(monst)) {
					autoID = true;
				}
				zapDamage(shootingMonst, monst, boltInView, &lightningColor, "lightning bolt", damage);
			}
			break;
		case BOLT_FORCE:
			if (monst && !(boltHasFlags & BOLT_NO_DAMAGE)) {
				if (canSeeMonster(monst)) {
					autoID = true;
				}
				zapDamage(shootingMonst, monst, boltInView, &forceFieldColor, "force bolt", damage);
			}
			break;
		case BOLT_TUNNELING:
			if ((boltHasFlags & BOLT_PENETRATING) && monst && !(boltHasFlags & BOLT_NO_DAMAGE)) { // penetrating tunnelling bolts do damage and can shatter reflecting monsters
				if (canSeeMonster(monst)) {
					autoID = true;
				}
				if ((monst->info.flags & MONST_REFLECT_4) && rand_percent(shatteringChance(boltLevel * power))) {
					zapDamage(shootingMonst, monst, boltInView, &wallCrystalColor, "disintergrating bolt", monst->currentHP);
				} else {
					zapDamage(shootingMonst, monst, boltInView, &wallCrystalColor, "disintergrating bolt", damage);
				}
			}
			break;			
		case BOLT_DETONATION:
			if (monst && !(monst->info.flags & MONST_INANIMATE)) {
				if (monst->info.DFType != DF_BOLT_DETONATION) { // this is equivalent to Brogue 1.7.2 explosive mutation, except missing the (explosive) side panel note
					monst->info.foreColor = &darkOrange;
					monst->info.flags |= MONST_FLITS; // more amusing
					monst->info.abilityFlags |= MA_DF_ON_DEATH;
					monst->info.defense /= 2;
					monst->info.maxHP /= 2;	// probably need some hackery to avoid this looking like it heals the monster
					if (monst->info.DFType == DF_ROT_GAS_PUFF) {
						monst->info.DFType = DF_METHANE_GAS_PUFF_ZOMBIE;
					} else {
						monst->info.DFType = DF_BOLT_DETONATION;
						monst->info.DFChance = 0;
					}
					if (monst->currentHP > monst->info.maxHP) {
						monst->currentHP = monst->info.maxHP;
					}
					if (!monst->status[STATUS_LIFESPAN_REMAINING]) {
						monst->status[STATUS_LIFESPAN_REMAINING] = rand_range(detonationLifeSpanMinimum(boltLevel / power), detonationLifeSpanMaximum(boltLevel / power));
						monst->maxStatus[STATUS_LIFESPAN_REMAINING] = monst->status[STATUS_LIFESPAN_REMAINING];
					}
					wakeUp(monst);
					refreshDungeonCell(x, y);
				}
				if ((boltHasFlags & BOLT_PENETRATING) && !(boltHasFlags & BOLT_NO_DAMAGE)) {
					if (canSeeMonster(monst)) {
						autoID = true;
					}
					if ((monst->info.flags & MONST_REFLECT_4) && rand_percent(shatteringChance(boltLevel * power))) {
						zapDamage(shootingMonst, monst, boltInView, &darkOrange, "explosive bolt", monst->currentHP);
					} else {
						zapDamage(shootingMonst, monst, boltInView, &darkOrange, "explosive bolt", damage);
					}
				}
			}
			break;
			
		case BOLT_POISON:
			if (monst && !(monst->info.flags & MONST_INANIMATE) && !monst->status[STATUS_POISON_IMMUNITY] && !(boltHasFlags & BOLT_NO_DAMAGE) &&
				(!(boltHasFlags & BOLT_PENETRATING) || zapDamage(shootingMonst, monst, boltInView, &lightningColor, "poisoned spine", damage))) {
				poisonDamage = boltPoison(boltLevel * power);
				monst->status[STATUS_POISONED] += poisonDamage;
				monst->maxStatus[STATUS_POISONED] = monst->info.maxHP;
				refreshSideBar(-1, -1, false);
				if (canSeeMonster(monst)) {
					flashMonster(monst, boltColors[BOLT_POISON], 100);
					autoID = true;
					if (monst != &player) {
						sprintf(buf, "%s %s %s sick",
								monstName,
								(monst == &player ? "feel" : "looks"),
								(monst->status[STATUS_POISONED] && !player.status[STATUS_HALLUCINATING] > monst->currentHP ? "fatally" : "very"));
						combatMessage(buf, messageColorFromVictim(monst));
					}
				}
			}
			break;
		case BOLT_FIRE:
			if (monst) {
				autoID = true;
				
				if (monst->status[STATUS_IMMUNE_TO_FIRE] > 0) {
					if (canSeeMonster(monst)) {
						sprintf(buf, "%s ignore%s %s firebolt",
								monstName,
								(monst == &player ? "" : "s"),
								canSeeMonster(shootingMonst) ? "the" : "a");
						combatMessage(buf, 0);
					}
				} else if (shieldBlocks(monst, i, damage, false)) {
					sprintf(buf, "%s block%s %s firebolt",
							monstName,
							(monst == &player ? "" : "s"),
							canSeeMonster(shootingMonst) ? "the" : "a");
					combatMessage(buf, 0);					
					// No further action. Note being immune to fire protects your shield against fire damage, but a shield block will
					// still result in you catching fire if you are in burnable terrain.
				} else if (!(boltHasFlags & BOLT_NO_DAMAGE) && zapDamage(shootingMonst, monst, boltInView, &orange, "firebolt", damage)) {
					exposeCreatureToFire(monst);
					refreshSideBar(-1, -1, false);
				}
			}
			exposeTileToFire(x, y, true); // burninate
			break;
		case BOLT_ENTRANCEMENT:
			if (monst && monst == &player) {
				flashMonster(monst, &confusionGasColor, 100);
				monst->maxStatus[STATUS_CONFUSED] = monst->status[STATUS_CONFUSED] = boltEntrancementDuration(boltLevel * power);
				refreshSideBar(-1, -1, false);
				message("the bolt hits you and you suddently feel disoriented.", true);
				autoID = true;
			} else if (monst && !(monst->info.flags & MONST_INANIMATE)) {
				monst->status[STATUS_ENTRANCED] = monst->maxStatus[STATUS_ENTRANCED] = boltEntrancementDuration(boltLevel * power);
				if (boltHasFlags & BOLT_FROM_WAND) {
					monst->status[STATUS_ENTRANCED_BY_WAND] = monst->maxStatus[STATUS_ENTRANCED_BY_WAND] = boltEntrancementDuration(boltLevel * power);
				}
				refreshSideBar(-1, -1, false);
				wakeUp(monst);
				if (canSeeMonster(monst)) {
					flashMonster(monst, boltColors[BOLT_ENTRANCEMENT], 100);
					autoID = true;
					sprintf(buf, "%s is entranced!", monstName);
					message(buf, false);
				}
			}
			break;
		case BOLT_HEALING:
			if (monst) {
				heal(monst, boltLevel * power * 10);
				refreshSideBar(-1, -1, false);
				if (canSeeMonster(monst)) {
					autoID = true;
				}
			}
			break;
		case BOLT_PLENTY:
			if (monst && !(monst->info.flags & MONST_INANIMATE) && !(monst->bookkeepingFlags & MONST_BOLT_CHAINED)) { // prevent player chain cloning in corridors
				newMonst = cloneMonster(monst, true, true);
				if (newMonst) {
					if (boltHasFlags & BOLT_FROM_WAND) {
						newMonst->currentHP = (newMonst->currentHP + 1) / 2;
						monst->currentHP = (monst->currentHP + 1) / 2;
					}
					if ((boltHasFlags & BOLT_FROM_STAFF) && !newMonst->status[STATUS_LIFESPAN_REMAINING]) {
						newMonst->status[STATUS_LIFESPAN_REMAINING] = newMonst->maxStatus[STATUS_LIFESPAN_REMAINING] = staffPlentyDuration(boltLevel * power);
					}
					if (boltHasFlags & (BOLT_FROM_WAND | BOLT_FROM_STAFF)) { // paranoia: we don't clear this flag for monster bolts
						newMonst->bookkeepingFlags |= MONST_BOLT_CHAINED;
					}
					refreshSideBar(-1, -1, false);
					flashMonster(monst, boltColors[BOLT_PLENTY], 100);
					flashMonster(newMonst, boltColors[BOLT_PLENTY], 100);
					autoID = true;
				}
			}
			break;
		case BOLT_DISCORD:
			if (monst && !(monst->info.flags & MONST_INANIMATE)) {
				monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = max(boltDiscordDuration(boltLevel * power), monst->status[STATUS_DISCORDANT]);
				if (canSeeMonster(monst)) {
					flashMonster(monst, boltColors[BOLT_DISCORD], 100);
					autoID = true;
				}
			}
			break;
		case BOLT_CONJURATION:
			for (j = 0; j < (boltBladeCount(boltLevel * power)); j++) {
				getQualifyingLocNear(newLoc, x, y, true, 0,
									 T_PATHING_BLOCKER & ~(T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT),
									 (HAS_PLAYER | HAS_MONSTER), false, false);
				monst = generateMonster(MK_SPECTRAL_BLADE, true);
				monst->xLoc = newLoc[0];
				monst->yLoc = newLoc[1];
				monst->bookkeepingFlags |= (MONST_FOLLOWER | MONST_BOUND_TO_LEADER | MONST_DOES_NOT_TRACK_LEADER);
				monst->bookkeepingFlags &= ~MONST_JUST_SUMMONED;
				monst->leader = &player;
				monst->creatureState = MONSTER_ALLY;
				monst->ticksUntilTurn = monst->info.attackSpeed + 1; // So they don't move before the player's next turn.
				pmap[monst->xLoc][monst->yLoc].flags |= HAS_MONSTER;
				refreshDungeonCell(monst->xLoc, monst->yLoc);
				//fadeInMonster(monst);
			}
			refreshSideBar(-1, -1, false);
			monst = NULL;
			autoID = true;
			break;
		case BOLT_SHIELDING:
			if (monst) {
				monst->protectionAmount = max(monst->protectionAmount, boltSentryProtection(boltLevel * power));
				monst->status[STATUS_SHIELDED] = min(20, monst->protectionAmount);
				monst->maxStatus[STATUS_SHIELDED] = monst->status[STATUS_SHIELDED];
				flashMonster(monst, boltColors[BOLT_SHIELDING], 100);
				autoID = true;
			}
			break;
		case BOLT_REFLECTION:
			if (monst) {
				monst->status[STATUS_REFLECTIVE] = monst->maxStatus[STATUS_REFLECTIVE] = boltReflectionDuration(boltLevel * power);
				refreshSideBar(-1, -1, false);
				// wakeUp(monst);
				if (canSeeMonster(monst)) {
					flashMonster(monst, boltColors[BOLT_REFLECTION], 100);
					autoID = true;
					sprintf(buf, "%s %s reflective!", monstName, monst == &player ? "are" : "is");
					message(buf, false);
				}
			}
			break;
		default:
			break;
	}
	
	return autoID;
}

// Relies on the sidebar entity list. If one is already selected, select the next qualifying. Otherwise, target the first qualifying.
boolean nextTargetAfter(short *returnX,
                        short *returnY,
                        short targetX,
                        short targetY,
                        boolean targetEnemies,
                        boolean targetAllies,
                        boolean targetItems,
                        boolean requireOpenPath) {
    short i, n;
    short selectedIndex = -1;
    creature *monst;
    item *theItem;
    
    for (i=0; i<ROWS; i++) {
        if (rogue.sidebarLocationList[i][0] == targetX
            && rogue.sidebarLocationList[i][1] == targetY
            && (i == ROWS - 1 || rogue.sidebarLocationList[i+1][0] != targetX || rogue.sidebarLocationList[i+1][1] != targetY)) {
            
            selectedIndex = i;
            break;
        }
    }
    
    for (i=1; i<=ROWS; i++) {
        n = (selectedIndex + i) % ROWS;
        targetX = rogue.sidebarLocationList[n][0];
        targetY = rogue.sidebarLocationList[n][1];
        if (targetX != -1
            && (targetX != player.xLoc || targetY != player.yLoc)
            && (!requireOpenPath || openPathBetween(player.xLoc, player.yLoc, targetX, targetY))) {

#ifdef BROGUE_ASSERTS
            assert(coordinatesAreInMap(targetX, targetY));
#endif

            monst = monsterAtLoc(targetX, targetY);
            if (monst) {
                
                if (monstersAreEnemies(&player, monst)) {
                    if (targetEnemies) {
                        *returnX = targetX;
                        *returnY = targetY;
                        return true;
                    }
                } else {
                    if (targetAllies) {
                        *returnX = targetX;
                        *returnY = targetY;
                        return true;
                    }
                }
            }
            
            theItem = itemAtLoc(targetX, targetY);
            if (!monst && theItem && targetItems) {
                *returnX = targetX;
                *returnY = targetY;
                return true;
            }
        }
    }
    return false;
}
    
//	creature *currentTarget, *monst, *returnMonst = NULL;
//	short currentDistance, shortestDistance;
//	
//	currentTarget = monsterAtLoc(targetX, targetY);
//	
//	if (!currentTarget || currentTarget == &player) {
//		currentTarget = monsters;
//		currentDistance = 0;
//	} else {
//		currentDistance = distanceBetween(player.xLoc, player.yLoc, targetX, targetY);
//	}
//	
//	// first try to find a monster with the same distance later in the chain.
//	for (monst = currentTarget->nextCreature; monst != NULL; monst = monst->nextCreature) {
//		if (distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc) == currentDistance
//			&& canSeeMonster(monst)
//			&& (targetAllies == (monst->creatureState == MONSTER_ALLY || (monst->bookkeepingFlags & MONST_CAPTIVE)))
//			&& (!requireOpenPath || openPathBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc))) {
//			
//			// got one!
//			returnMonst = monst;
//			break;
//		}
//	}
//	
//	if (!returnMonst) {
//		// okay, instead pick the qualifying monster (excluding the current target)
//		// with the shortest distance greater than currentDistance.
//		shortestDistance = max(DCOLS, DROWS);
//		for (monst = currentTarget->nextCreature;; monst = monst->nextCreature) {
//			if (monst == NULL) {
//				monst = monsters;
//			} else if (distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc) < shortestDistance
//					   && distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc) > currentDistance
//					   && canSeeMonster(monst)
//					   && (targetAllies == (monst->creatureState == MONSTER_ALLY || (monst->bookkeepingFlags & MONST_CAPTIVE)))
//					   && (!requireOpenPath || openPathBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc))) {
//				// potentially this one
//				shortestDistance = distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc);
//				returnMonst = monst;
//			}
//			if (monst == currentTarget) {
//				break;
//			}
//		}
//	}
//	
//	if (!returnMonst) {
//		// okay, instead pick the qualifying monster (excluding the current target)
//		// with the shortest distance period.
//		shortestDistance = max(DCOLS, DROWS);
//		for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
//			if (distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc) < shortestDistance
//				&& canSeeMonster(monst)
//				&& (targetAllies == (monst->creatureState == MONSTER_ALLY || (monst->bookkeepingFlags & MONST_CAPTIVE)))
//				&& (!requireOpenPath || openPathBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc))) {
//				// potentially this one
//				shortestDistance = distanceBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc);
//				returnMonst = monst;
//			}
//		}
//	}
//	
//	if (returnMonst) {
//		plotCharWithColor(returnMonst->info.displayChar, mapToWindowX(returnMonst->xLoc),
//						  mapToWindowY(returnMonst->yLoc), *(returnMonst->info.foreColor), white);
//	}
//	
//	return returnMonst;
//}

// Returns how far it went before hitting something.
short hiliteTrajectory(short coordinateList[DCOLS][2], short numCells, boolean eraseHiliting, boolean passThroughMonsters) {
	short x, y, i;
	creature *monst;
    
	for (i=0; i<numCells; i++) {
		x = coordinateList[i][0];
		y = coordinateList[i][1];
		if (eraseHiliting) {
			refreshDungeonCell(x, y);
		} else {
			//hiliteCell(x, y, &hiliteColor, 50, true); // yellow
			hiliteCell(x, y, &red, 50, true); // red
		}
		
		if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
			|| pmap[x][y].flags & (HAS_PLAYER)) {
			i++;
			break;
		} else if (!(pmap[x][y].flags & DISCOVERED)) {
			break;
		} else if (!passThroughMonsters && (pmap[x][y].flags & (HAS_MONSTER))
				   && (playerCanSee(x, y) || player.status[STATUS_TELEPATHIC])) {
			monst = monsterAtLoc(x, y);
			if (!(monst->bookkeepingFlags & MONST_SUBMERGED)
				&& (!monst->status[STATUS_INVISIBLE] || player.status[STATUS_TELEPATHIC])) {
                
				i++;
				break;
			}
		}
	}
	return i;
}

// Event is optional. Returns true if the event should be executed by the parent function.
boolean moveCursor(boolean *targetConfirmed,
				   boolean *canceled,
				   boolean *tabKey,
				   short targetLoc[2],
				   rogueEvent *event,
				   buttonState *state,
				   boolean colorsDance,
				   boolean keysMoveCursor,
				   boolean targetCanLeaveMap) {
	signed long keystroke;
	short moveIncrement;
	short buttonInput;
	boolean cursorMovementCommand, again, movementKeystroke, sidebarHighlighted;
	rogueEvent theEvent;
	
	short *cursor = rogue.cursorLoc; // shorthand
	
	cursor[0] = targetLoc[0];
	cursor[1] = targetLoc[1];
	
	*targetConfirmed = *canceled = *tabKey = false;
	sidebarHighlighted = false;
	
	do {
		again = false;
		cursorMovementCommand = false;
		movementKeystroke = false;
		
		assureCosmeticRNG;
		
		if (state) { // Also running a button loop.
			
			// Update the display.
			overlayDisplayBuffer(state->dbuf, NULL);
			
			// Get input.
			nextBrogueEvent(&theEvent, false, colorsDance, true);
			
			// Process the input.
			buttonInput = processButtonInput(state, NULL, &theEvent);
			
			if (buttonInput != -1) {
				state->buttonDepressed = state->buttonFocused = -1;
				drawButtonsInState(state);
			}
			
			// Revert the display.
			overlayDisplayBuffer(state->rbuf, NULL);
			
		} else { // No buttons to worry about.
			nextBrogueEvent(&theEvent, false, colorsDance, true);
		}
		restoreRNG;
		
		if (theEvent.eventType == MOUSE_UP || theEvent.eventType == MOUSE_ENTERED_CELL) {
			if (theEvent.param1 >= 0
				&& theEvent.param1 < mapToWindowX(0)
				&& theEvent.param2 >= 0
				&& theEvent.param2 < ROWS - 1
				&& rogue.sidebarLocationList[theEvent.param2][0] > -1) {
				
				// If the cursor is on an entity in the sidebar.
				cursor[0] = rogue.sidebarLocationList[theEvent.param2][0];
				cursor[1] = rogue.sidebarLocationList[theEvent.param2][1];
				sidebarHighlighted = true;
				cursorMovementCommand = true;
				refreshSideBar(cursor[0], cursor[1], false);
				if (theEvent.eventType == MOUSE_UP) {
					*targetConfirmed = true;
				}
			} else if (coordinatesAreInMap(windowToMapX(theEvent.param1), windowToMapY(theEvent.param2))
				|| targetCanLeaveMap && theEvent.eventType != MOUSE_UP) {
				
				// If the cursor is in the map area, or is allowed to leave the map and it isn't a click.
				if (theEvent.eventType == MOUSE_UP
					&& !theEvent.shiftKey
					&& (theEvent.controlKey || (cursor[0] == windowToMapX(theEvent.param1) && cursor[1] == windowToMapY(theEvent.param2)))) {
					
					*targetConfirmed = true;
				}
				cursor[0] = windowToMapX(theEvent.param1);
				cursor[1] = windowToMapY(theEvent.param2);
				cursorMovementCommand = true;
			} else {
				cursorMovementCommand = false;
				again = theEvent.eventType != MOUSE_UP;
			}
		} else if (theEvent.eventType == KEYSTROKE) {
			keystroke = theEvent.param1;
			moveIncrement = ( (theEvent.controlKey || theEvent.shiftKey) ? 5 : 1 );
			stripShiftFromMovementKeystroke(&keystroke);
			switch(keystroke) {
				case LEFT_ARROW:
				case LEFT_KEY:
				case NUMPAD_4:
					if (keysMoveCursor && cursor[0] > 0) {
						cursor[0] -= moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case RIGHT_ARROW:
				case RIGHT_KEY:
				case NUMPAD_6:
					if (keysMoveCursor && cursor[0] < DCOLS - 1) {
						cursor[0] += moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case UP_ARROW:
				case UP_KEY:
				case NUMPAD_8:
					if (keysMoveCursor && cursor[1] > 0) {
						cursor[1] -= moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case DOWN_ARROW:
				case DOWN_KEY:
				case NUMPAD_2:
					if (keysMoveCursor && cursor[1] < DROWS - 1) {
						cursor[1] += moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case UPLEFT_KEY:
				case NUMPAD_7:
					if (keysMoveCursor && cursor[0] > 0 && cursor[1] > 0) {
						cursor[0] -= moveIncrement;
						cursor[1] -= moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case UPRIGHT_KEY:
				case NUMPAD_9:
					if (keysMoveCursor && cursor[0] < DCOLS - 1 && cursor[1] > 0) {
						cursor[0] += moveIncrement;
						cursor[1] -= moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case DOWNLEFT_KEY:
				case NUMPAD_1:
					if (keysMoveCursor && cursor[0] > 0 && cursor[1] < DROWS - 1) {
						cursor[0] -= moveIncrement;
						cursor[1] += moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case DOWNRIGHT_KEY:
				case NUMPAD_3:
					if (keysMoveCursor && cursor[0] < DCOLS - 1 && cursor[1] < DROWS - 1) {
						cursor[0] += moveIncrement;
						cursor[1] += moveIncrement;
					}
					cursorMovementCommand = movementKeystroke = keysMoveCursor;
					break;
				case TAB_KEY:
				case NUMPAD_0:
					*tabKey = true;
					break;
				case RETURN_KEY:
				case ENTER_KEY:
					*targetConfirmed = true;
					break;
				case ESCAPE_KEY:
				case ACKNOWLEDGE_KEY:
					*canceled = true;
					break;
				default:
					break;
			}
		} else if (theEvent.eventType == RIGHT_MOUSE_UP) {
			// do nothing
		} else {
			again = true;
		}
		
		if (sidebarHighlighted
			&& (!(pmap[cursor[0]][cursor[1]].flags & (HAS_PLAYER | HAS_MONSTER))
								   || !canSeeMonster(monsterAtLoc(cursor[0], cursor[1])))
			&& (!(pmap[cursor[0]][cursor[1]].flags & HAS_ITEM)) || (playerCanSeeOrSense(cursor[0], cursor[1]))) {
			
			// The sidebar is highlighted but the cursor is not on a visible item or monster. Un-highlight the sidebar.
			refreshSideBar(-1, -1, false);
			sidebarHighlighted = false;
		}
		
		if (targetCanLeaveMap && !movementKeystroke) {
			// permit it to leave the map by up to 1 space in any direction if mouse controlled.
			cursor[0] = clamp(cursor[0], -1, DCOLS);
			cursor[1] = clamp(cursor[1], -1, DROWS);
		} else {
			cursor[0] = clamp(cursor[0], 0, DCOLS - 1);
			cursor[1] = clamp(cursor[1], 0, DROWS - 1);
		}
	} while (again && (!event || !cursorMovementCommand));
	
	if (event) {
		*event = theEvent;
	}
	
	if (sidebarHighlighted) {
		// Don't leave the sidebar highlighted when we exit.
		refreshSideBar(-1, -1, false);
		sidebarHighlighted = false;
	}
	
	targetLoc[0] = cursor[0];
	targetLoc[1] = cursor[1];
	
	return !cursorMovementCommand;
}

void pullMouseClickDuringPlayback(short loc[2]) {
	rogueEvent theEvent;
	
#ifdef BROGUE_ASSERTS
	assert(rogue.playbackMode);
#endif
	nextBrogueEvent(&theEvent, false, false, false);
	loc[0] = windowToMapX(theEvent.param1);
	loc[1] = windowToMapY(theEvent.param2);
}

// Return true if a target is chosen, or false if canceled.
boolean chooseTarget(short returnLoc[2],
					 short maxDistance,
					 boolean stopAtTarget,
					 boolean autoTarget,
					 boolean targetAllies,
					 boolean passThroughCreatures) {
	short originLoc[2], targetLoc[2], oldTargetLoc[2], coordinates[DCOLS][2], numCells, i, distance, newX, newY;
	creature *monst;
	boolean canceled, targetConfirmed, tabKey, cursorInTrajectory, focusedOnMonster = false;
	rogueEvent event;
		
	if (rogue.playbackMode) {
		// In playback, pull the next event (a mouseclick) and use that location as the target.
		pullMouseClickDuringPlayback(returnLoc);
		rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
		return true;
	}
	
	assureCosmeticRNG;
	
	originLoc[0] = player.xLoc;
	originLoc[1] = player.yLoc;
	
	targetLoc[0] = oldTargetLoc[0] = player.xLoc;
	targetLoc[1] = oldTargetLoc[1] = player.yLoc;
	
	if (autoTarget) {
	
		if (rogue.lastTarget
			&& canSeeMonster(rogue.lastTarget)
			&& (targetAllies == (rogue.lastTarget->creatureState == MONSTER_ALLY))
			&& rogue.lastTarget->depth == rogue.depthLevel
			&& !(rogue.lastTarget->bookkeepingFlags & MONST_IS_DYING)
			&& openPathBetween(player.xLoc, player.yLoc, rogue.lastTarget->xLoc, rogue.lastTarget->yLoc)) {
			
			monst = rogue.lastTarget;
		} else {
			//rogue.lastTarget = NULL;
			if (nextTargetAfter(&newX, &newY, targetLoc[0], targetLoc[1], !targetAllies, targetAllies, false, true)) {
                targetLoc[0] = newX;
                targetLoc[1] = newY;
            }
            monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
		}
		if (monst) {
			targetLoc[0] = monst->xLoc;
			targetLoc[1] = monst->yLoc;
			refreshSideBar(monst->xLoc, monst->yLoc, false);
			focusedOnMonster = true;
		}
	}
	
	numCells = getLineCoordinates(coordinates, originLoc, targetLoc);
	if (maxDistance > 0) {
		numCells = min(numCells, maxDistance);
	}
	if (stopAtTarget) {
		numCells = min(numCells, distanceDiagonal(player.xLoc, player.yLoc, targetLoc[0], targetLoc[1]));
	}
	
	targetConfirmed = canceled = tabKey = false;
	
	do {
		printLocationDescription(targetLoc[0], targetLoc[1]);
		
		if (canceled) {
			refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);
			hiliteTrajectory(coordinates, numCells, true, passThroughCreatures);
			confirmMessages();
			rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
			restoreRNG;
			return false;
		}
		
		if (tabKey) {
			if (nextTargetAfter(&newX, &newY, targetLoc[0], targetLoc[1], !targetAllies, targetAllies, false, true)) {
                targetLoc[0] = newX;
                targetLoc[1] = newY;
            }
		}
		
		monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
		if (monst != NULL && monst != &player && canSeeMonster(monst)) {
			refreshSideBar(monst->xLoc, monst->yLoc, false);
			focusedOnMonster = true;
		} else if (focusedOnMonster) {
			refreshSideBar(-1, -1, false);
			focusedOnMonster = false;
		}
		
		refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);
		
		hiliteTrajectory(coordinates, numCells, true, passThroughCreatures);
		
		if (!targetConfirmed) {
			numCells = getLineCoordinates(coordinates, originLoc, targetLoc);
			if (maxDistance > 0) {
				numCells = min(numCells, maxDistance);
			}
			
			if (stopAtTarget) {
				numCells = min(numCells, distanceDiagonal(player.xLoc, player.yLoc, targetLoc[0], targetLoc[1]));
			}
			distance = hiliteTrajectory(coordinates, numCells, false, passThroughCreatures);
			cursorInTrajectory = false;
			for (i=0; i<distance; i++) {
				if (coordinates[i][0] == targetLoc[0] && coordinates[i][1] == targetLoc[1]) {
					cursorInTrajectory = true;
					break;
				}
			}
			hiliteCell(targetLoc[0], targetLoc[1], &white, (cursorInTrajectory ? 100 : 35), true);	
		}
		
		oldTargetLoc[0] = targetLoc[0];
		oldTargetLoc[1] = targetLoc[1];
		moveCursor(&targetConfirmed, &canceled, &tabKey, targetLoc, &event, NULL, false, true, false);
		if (event.eventType == RIGHT_MOUSE_UP) { // Right mouse cancels.
			canceled = true;
		}
	} while (!targetConfirmed);
	if (maxDistance > 0) {
		numCells = min(numCells, maxDistance);
	}
	hiliteTrajectory(coordinates, numCells, true, passThroughCreatures);
	refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);
	
	if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
		confirmMessages();
		restoreRNG;
		rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
		return false;
	}
	
	monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
	if (monst && monst != &player && canSeeMonster(monst)) {
		rogue.lastTarget = monst;
	}
	
	returnLoc[0] = targetLoc[0];
	returnLoc[1] = targetLoc[1];
	restoreRNG;
	rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
	return true;
}

void identifyItemKind(item *theItem) {
    itemTable *theTable;
	short tableCount, i, lastItem;
    
    theTable = tableForItemCategory(theItem->category);
    if (theTable) {
		theItem->flags &= ~ITEM_KIND_AUTO_ID;
        
        tableCount = 0;
        lastItem = -1;
        
        switch (theItem->category) {
            case SCROLL:
                tableCount = NUMBER_SCROLL_KINDS;
                break;
            case POTION:
                tableCount = NUMBER_POTION_KINDS;
                break;
            case WAND:
                tableCount = NUMBER_WAND_KINDS;
                break;
            case STAFF:
                tableCount = NUMBER_STAFF_KINDS;
                break;
            case RING:
                tableCount = NUMBER_RING_KINDS;
                break;
            default:
                break;
        }
        if ((theItem->category & RING)
            && theItem->enchant1 <= 0) {
            
            theItem->flags |= ITEM_IDENTIFIED;
        }
        if (tableCount) {
            theTable[theItem->kind].identified = true;
            for (i=0; i<tableCount; i++) {
                if (!(theTable[i].identified)) {
                    if (lastItem != -1) {
                        return; // at least two unidentified items remain
                    }
                    lastItem = i;
                }
            }
            if (lastItem != -1) {
                // exactly one unidentified item remains
                theTable[lastItem].identified = true;
            }
        }
    }
}

void autoIdentify(item *theItem) {
	short quantityBackup;
	char buf[COLS * 3], oldName[COLS * 3], newName[COLS * 3];
	
    if (tableForItemCategory(theItem->category)
        && !tableForItemCategory(theItem->category)[theItem->kind].identified) {
        
        identifyItemKind(theItem);
        quantityBackup = theItem->quantity;
        theItem->quantity = 1;
        itemName(theItem, newName, false, true, NULL);
        theItem->quantity = quantityBackup;
        sprintf(buf, "(It must %s %s.)",
                ((theItem->category & (POTION | SCROLL)) ? "have been" : "be"),
                newName);
        messageWithColor(buf, &itemMessageColor, false);
    }
    
    if ((theItem->category & (WEAPON | ARMOR))
        && (theItem->flags & ITEM_RUNIC)
        && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)) {
        
        itemName(theItem, oldName, false, false, NULL);
        theItem->flags |= ITEM_RUNIC_IDENTIFIED;
        itemName(theItem, newName, true, true, NULL);
        sprintf(buf, "(Your %s must be %s.)", oldName, newName);
        messageWithColor(buf, &itemMessageColor, false);
    }
}

// returns whether the item disappeared
// items now disappear if a special dart or a thrown runic weapon which hits and activates
boolean hitMonsterWithProjectileWeapon(creature *thrower, creature *monst, item *theItem) {
	char buf[DCOLS], theItemName[DCOLS], targetName[DCOLS], armorRunicString[DCOLS], shieldRunicString[DCOLS];
	boolean thrownWeaponHit;
	item *equippedWeapon;
	short damage;
	
	if (!(theItem->category & WEAPON)) {
		return false;
	}
	
	theItem->flags &= ~ITEM_PLAYER_AVOIDS; // Don't avoid thrown weapons.
	
	armorRunicString[0] = '\0';
	shieldRunicString[0] = '\0';
	
	itemName(theItem, theItemName, false, false, NULL);
	monsterName(targetName, monst, true);

	if (!monst->status[STATUS_ENTRANCED_BY_WAND]) {
		monst->status[STATUS_ENTRANCED] = 0;
	}
	
	if (monst != &player
		&& monst->creatureMode != MODE_PERM_FLEEING
		&& (monst->creatureState != MONSTER_FLEEING || monst->status[STATUS_MAGICAL_FEAR])
		&& !(monst->bookkeepingFlags & MONST_CAPTIVE)) {
		monst->creatureState = MONSTER_TRACKING_SCENT;
		if (monst->status[STATUS_MAGICAL_FEAR]) {
			monst->status[STATUS_MAGICAL_FEAR] = 1;
		}
	}
	
	if (thrower == &player) {
		equippedWeapon = rogue.weapon;
		equipItem(theItem, true);
		if (rogue.weapon->vorpalEnemy == monst->info.slayID) {
			thrownWeaponHit = true;
		}
		else {
			thrownWeaponHit = attackHit(&player, monst);
		}
		if (equippedWeapon) {
			equipItem(equippedWeapon, true);
		} else {
			unequipItem(theItem, true);
		}
	} else {
		thrownWeaponHit = attackHit(thrower, monst);
	}
	
	if (thrownWeaponHit) {
		damage = (monst->info.flags & MONST_IMMUNE_TO_WEAPONS ? 0 :
				  randClump(theItem->damage)) * pow(WEAPON_ENCHANT_DAMAGE_FACTOR, netEnchant(theItem));
		
		if ((monst->info.flags & MONST_FIERY) && theItem->kind == EXTINGUISHING_DART) {
			damage *= 3;
		}
		
		if (monst == &player) {
			applyArmorRunicEffect(rogue.armor, armorRunicString, thrower, &damage, false);
			applyArmorRunicEffect(rogue.shield, shieldRunicString, thrower, &damage, false);
		}
		
		if (theItem->kind == MARKING_DART) {
			sprintf(buf, "the %s marked %s.", theItemName, targetName);
			messageWithColor(buf, messageColorFromVictim(monst), false);
			monst->status[STATUS_MARKED] = monst->maxStatus[STATUS_MARKED] = 1000;
			rogue.markedMonsters++;
			updateTelepathy();
			displayLevel();
			return true;
		} else if (inflictDamage(monst, damage, &red)) { // monster killed
			sprintf(buf, "the %s %s %s.",
                    theItemName,
                    (monst->info.flags & MONST_INANIMATE) ? "destroyed" : "killed",
                    targetName);
			messageWithColor(buf, messageColorFromVictim(monst), false);
			return false;
		} else {
			sprintf(buf, "the %s hit %s.", theItemName, targetName);
			if ((theItem->flags & ITEM_RUNIC)
				|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
				&& (theItem->hiddenRunicEnchantsRequired <= 0))) {
				magicWeaponHit(monst, theItem, false, true);
			}
			messageWithColor(buf, messageColorFromVictim(monst), false);
			
			switch(theItem->kind)
			{
				case POISON_DART:
					if (damage > 0) {
						if (monst == &player && !player.status[STATUS_POISONED]) {
							combatMessage("scalding poison fills your veins", &badMessageColor);
						}
						if (!monst->status[STATUS_POISONED]) {
							monst->maxStatus[STATUS_POISONED] = 0;
						}
						monst->status[STATUS_POISONED] = max(monst->status[STATUS_POISONED], damage * 4); // darts have a high poison to initial damage ratio
						monst->maxStatus[STATUS_POISONED] = monst->info.maxHP;
					}
					break;
				case DISCORD_DART:
					monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = max(damage * 4, monst->status[STATUS_DISCORDANT]);
					break;
				case CONFUSING_DART:
					monst->status[STATUS_CONFUSED] = max(monst->status[STATUS_CONFUSED], damage * 2);
					monst->maxStatus[STATUS_CONFUSED] = monst->status[STATUS_CONFUSED];
					break;
				case DARKNESS_DART:
					break;
				case EXTINGUISHING_DART:
					extinguishFireOnCreature(monst);
					break;
				case WEAKNESS_DART:
					if (damage > 0) {
						weaken(monst, 60);
					}
					break;
				case SLOWING_DART:
					slow(monst, damage * 2);
					break;
			}
			
			moralAttack(thrower, monst);
		}
		if (armorRunicString[0]) {
			message(armorRunicString, false);
		}
		if (shieldRunicString[0]) {
			message(shieldRunicString, false);
		}
		if (theItem->kind >= DART && (((theItem->flags & ITEM_RUNIC)
			&& (theItem->enchant2 != W_SLAYING || theItem->vorpalEnemy == monst->info.slayID))
				|| (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
				&& (theItem->hiddenRunicEnchantsRequired <= 0)
					&& (theItem->hiddenRunic != W_SLAYING || theItem->vorpalEnemy == monst->info.slayID)))) { // runic ranged weapons always lost if they hit
			return true;
		}
		if (theItem->kind > DART && theItem->kind < JAVELIN // special darts always lost
			&& theItem->kind != INCENDIARY_DART && theItem->kind != EXTINGUISHING_DART && theItem->kind != DARKNESS_DART) {
			// but note some special darts will be lost later to allow their special effect to apply
			return true;
		}
		return false;
	} else {
		sprintf(buf, "the %s missed %s.", theItemName, targetName);
		message(buf, false);
		return false;
	}
}

boolean explodeItem(item *theItem, short x, short y, const char *name, const char *verb)
{
	short i, j;
	short **grid;
	char buf[COLS*3];
	boolean elixir = (theItem->category & ELIXIR) != 0;
	
	if (elixir && cellHasTerrainFlag(x, y, (T_AUTO_DESCENT | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_IS_DF_TRAP)) && theItem->kind != ELIXIR_DARKNESS) {
		elixir = false;
	}
	
	switch (theItem->kind) {
		case POTION_POISON:
			sprintf(buf, "the %s %s and a deadly purple cloud billows out!", name, verb);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = GAS_TRAP_POISON;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
			message(buf, false);
			break;
		case POTION_CONFUSION:
			sprintf(buf, "the %s %s and a multi-hued cloud billows out!", name, verb);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = GAS_TRAP_CONFUSION;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
			message(buf, false);
			break;
		case POTION_PARALYSIS:
			sprintf(buf, "the %s %s and a cloud of pink gas billows out!", name, verb);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = GAS_TRAP_PARALYSIS;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
			message(buf, false);
			break;
		case POTION_INCINERATION:
			sprintf(buf, "the %s %s and its contents burst violently into flame!", name, verb);
			message(buf, false);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = FLAMETHROWER;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
			break;
		case POTION_DARKNESS:
			sprintf(buf, "the %s %s and the lights in the area %s.", name, verb, elixir ? "brighten" : "start fading");
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[elixir ? DF_LIGHT_ELIXIR : DF_DARKNESS_POTION], true, false);
			message(buf, false);
			break;
		case POTION_DESCENT:
			sprintf(buf, "as the %s %s, the ground vanishes!", name, verb);
			message(buf, false);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = TRAP_DOOR_HIDDEN;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
			break;
		case POTION_LICHEN:
			sprintf(buf, "the %s %s and deadly spores spill out!", name, verb);
			message(buf, false);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = CREEPER_FUNGUS;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
			break;
		case POTION_WATER:
			sprintf(buf, "the %s %s and water floods the area!", name, verb);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = FLOOD_TRAP;
			}
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DEEP_WATER_POTION], true, false);
			message(buf, false);
			break;
		case POTION_WINDS:
			sprintf(buf, "strong winds swirl out from the %s!", name);
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_WINDS_POTION], true, false);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = WIND_VENT;
			}
			message(buf, false);
			break;
		case POTION_STENCH:
			sprintf(buf, "the %s %s and the air is filled with a hideous stench!", name, verb);
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ROT_GAS_CLOUD_POTION], true, false);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = STINKFRUIT_STALK;				
				spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_STINKFRUIT_PODS_GROW_INITIAL], true, false);
			}			
			message(buf, false);
			break;
		case POTION_EXPLOSION:
			if (theItem->category & POTION) {
				sprintf(buf, "the contents of the %s explodes violently!", name);
				spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_POTION_EXPLOSION], true, false);
				message(buf, false);
				grid = allocDynamicGrid();
				fillDynamicGrid(grid, 0);
				
				calculateDistances(grid, x, y, T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION, NULL, true, false, true);
				
				for (i=0; i<DCOLS; i++) {
					for (j=0; j<DROWS; j++) {
						if (grid[i][j] >= 0 && grid[i][j] < 5) {
							if (tileCatalog[pmap[i][j].layers[DUNGEON]].displayChar == STATUE_CHAR) {
								pmap[i][j].layers[DUNGEON] = FLOOR;
								spawnDungeonFeature(i, j, &dungeonFeatureCatalog[DF_RUBBLE], true, false);
							}
						}
					}
				}
				freeDynamicGrid(grid);
			} else {
				if (theItem->category & ELIXIR) {
					pmap[x][y].layers[SURFACE] = INERT_BRIMSTONE;
				}				
				spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_METHANE_GAS_CLOUD_POTION], true, false);
				sprintf(buf, "an explosive gas escapes from the %s!", name);
				message(buf, false);
				break;	
			}
			break;
		case POTION_FIRE_IMMUNITY:
			sprintf(buf, "the %s %s and a fire suppressive gas is released!", name, verb);
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_FIRE_EXTINGUISHING_POTION], true, false);
			message(buf, false);
			break;			
		case POTION_LIFE:
			sprintf(buf, "the %s %s and cloud of healing gas is released!", name, verb);
			if (theItem->category & ELIXIR) {
				pmap[x][y].layers[SURFACE] = BLOODFLOWER_STALK;
				spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_BLOODFLOWER_PODS_GROW_INITIAL], true, false);
			}			
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_HEALING_POTION], true, false);
			message(buf, false);
			break;			
		default:
			return false;
	}
	return true;
}

void throwItem(item *theItem, creature *thrower, short targetLoc[2], short maxDistance) {
	short listOfCoordinates[MAX_BOLT_LENGTH][2], originLoc[2];
	short i, j, x, y, numCells, hitCount;
	creature *hitList[8] = {NULL};
	char buf[COLS*3], buf2[COLS], buf3[COLS];
	uchar displayChar;
	color foreColor, backColor, multColor;
	short dropLoc[2];
	boolean hitSomethingSolid = false, fastForward = false, loseItem = false;
	
	theItem->flags |= ITEM_PLAYER_AVOIDS; // Avoid thrown items, unless it's a weapon.
	
	x = originLoc[0] = thrower->xLoc;
	y = originLoc[1] = thrower->yLoc;
	
	numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc);
	
	if ((theItem->category & WEAPON) && (theItem->flags & ITEM_ATTACKS_SLOWLY)) {
		thrower->ticksUntilTurn += 2 * player.attackSpeed;
	} else if ((theItem->category & WEAPON) && (theItem->flags & ITEM_ATTACKS_QUICKLY)) {
		thrower->ticksUntilTurn += player.attackSpeed / 2;
	} else {
		thrower->ticksUntilTurn += player.attackSpeed;
	}
	
	if (thrower != &player && (pmap[originLoc[0]][originLoc[1]].flags & IN_FIELD_OF_VIEW)) {
		monsterName(buf2, thrower, true);
		itemName(theItem, buf3, false, true, NULL);
		sprintf(buf, "%s hurls %s.", buf2, buf3);
		message(buf, false);
	}
	
	for (i=0; i<numCells && i < maxDistance; i++) {
		
		x = listOfCoordinates[i][0];
		y = listOfCoordinates[i][1];

		// Grappling darts leave chain on intervening grids.
		if ((theItem->category & WEAPON) && theItem->kind == GRAPPLING_DART) {
			spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_GRAPPLING], true, false);
		}
		
		if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {			
//			if (projectileReflects(thrower, monst) && i < DCOLS*2) {
//				if (projectileReflects(thrower, monst)) { // if it scores another reflection roll, reflect at caster
//					numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, true);
//				} else {
//					numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
//				}
//				
//				monsterName(buf2, monst, true);
//				itemName(theItem, buf3, false, false, NULL);
//				sprintf(buf, "%s deflect%s the %s", buf2, (defender == &player ? "" : "s"), buf3);
//				combatMessage(buf, 0);
//				continue;
//			}
			
			hitCount = makeHitList(hitList, theItem->flags, x, y, i ? tDirs[x-listOfCoordinates[i-1][0]+1][y-listOfCoordinates[i-1][1]+1] : tDirs[x-originLoc[0]+1][y-originLoc[1]+1]);

			// Attack!
			for (j=0; j<hitCount; j++) {
				if (hitList[j]
					&& !(hitList[j]->bookkeepingFlags & MONST_IS_DYING)) {
					loseItem |= (hitMonsterWithProjectileWeapon(thrower, hitList[j], theItem));
				}
			}
			
			if (loseItem) {
				return;
			}
			break;
		}
		
		// We hit something!
		if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) || cellHasTerrainFlag3(x, y, (T3_IS_WIND))) {
			if ((theItem->category & WEAPON)
				&& (theItem->kind == INCENDIARY_DART)
				&& (cellHasTerrainFlag(x, y, T_IS_FLAMMABLE) || (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)))) {
				// Incendiary darts thrown at flammable obstructions (foliage, wooden barricades, doors) will hit the obstruction
				// instead of bursting a cell earlier.
			} else {
				i--;
				if (i >= 0) {
					x = listOfCoordinates[i][0];
					y = listOfCoordinates[i][1];
				} else { // it was aimed point-blank into an obstruction
					x = thrower->xLoc;
					y = thrower->yLoc;
				}
			}
			hitSomethingSolid = true;
			break;
		}
		
		if (playerCanSee(x, y)) { // show the graphic
			getCellAppearance(x, y, &displayChar, &foreColor, &backColor);
			foreColor = *(theItem->foreColor);
			if (pmap[x][y].flags & VISIBLE) {
				colorMultiplierFromDungeonLight(x, y, &multColor);
				applyColorMultiplier(&foreColor, &multColor);
			} else { // clairvoyant visible
				applyColorMultiplier(&foreColor, &clairvoyanceColor);
			}
			plotCharWithColor(theItem->displayChar, mapToWindowX(x), mapToWindowY(y), foreColor, backColor);
			
			if (!fastForward) {
				fastForward = rogue.playbackFastForward || pauseBrogue(25);
			}
			
			refreshDungeonCell(x, y);
		}
		
		if (x == targetLoc[0] && y == targetLoc[1]) { // reached its target
			break;
		}
	}
	
	if ((theItem->category & CHARM) && (!theItem->charges || theItem->flags & ITEM_OVERCHARGED)
		&& theItem->kind != CHARM_HEALTH && theItem->kind != CHARM_FIRE_IMMUNITY) { // charms of healing, fire immunity don't create gas
		if (explodeItem(theItem, x, y, "charm", "activates")) {
			if (theItem->charges > 0) {
				theItem->flags &= ~ITEM_OVERCHARGED;
			} else {
				theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
			}
			//autoIdentify(theItem);
			
			refreshDungeonCell(x, y);
		}
	} else if ((theItem->category & (POTION | ELIXIR)) && (hitSomethingSolid || !cellHasTerrainFlag(x, y, T_AUTO_DESCENT))) {
		if (explodeItem(theItem, x, y, "flask", "shatters")) {
			autoIdentify(theItem);
			
			refreshDungeonCell(x, y);
			
			//if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
			//	defender = monsterAtLoc(x, y);
			//	applyInstantTileEffectsToCreature(monst);
			//}			
		} else {
			if (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
				strcpy(buf2, "against");
			} else if (tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].flags & T_STAND_IN_TILE) {
				strcpy(buf2, "into");
			} else {
				strcpy(buf2, "on");
			}
			sprintf(buf, "the flask shatters and %s liquid splashes harmlessly %s %s.",
					theItem->category == POTION ? potionTable[theItem->kind].flavor : elixirTable[theItem->kind].flavor, buf2, tileText(x, y));
			message(buf, false);
			if (theItem->kind == POTION_HALLUCINATION && (theItem->flags & ITEM_MAGIC_DETECTED)) {
				autoIdentify(theItem);
			}
		}
		
		return;
	}
	
	if ((theItem->category & WEAPON) && theItem->kind == INCENDIARY_DART) {
		spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_EXPLOSION], true, false);
		if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
			exposeCreatureToFire(monsterAtLoc(x, y));
		}
		return;
	}
	if ((theItem->category & WEAPON) && theItem->kind == GRAPPLING_DART) {
		return;
	}
	if ((theItem->category & WEAPON) && theItem->kind == DARKNESS_DART) {
		spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_DARKNESS], true, false);
		return;
	}
	if ((theItem->category & WEAPON) && theItem->kind == EXTINGUISHING_DART) {
		spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_EXTINGUISH], true, false);
		return;
	}
	getQualifyingLocNear(dropLoc, x, y, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
	placeItem(theItem, dropLoc[0], dropLoc[1]);
	refreshDungeonCell(dropLoc[0], dropLoc[1]);
}

void throwCommand(item *theItem) {
	item *thrownItem;
	char buf[COLS], theName[COLS];
	unsigned char command[10];
	short maxDistance, zapTarget[2], originLoc[2], quantity;
	boolean autoTarget;
	
	command[0] = THROW_KEY;
	if (theItem == NULL) {
		theItem = promptForItemOfType((ALL_ITEMS), 0, 0, "Throw what? (a-z, shift for more info; or <esc> to cancel)", true);
	}
	if (theItem == NULL) {
		return;
	}
	
	quantity = theItem->quantity;
	theItem->quantity = 1;
	itemName(theItem, theName, false, false, NULL);
	theItem->quantity = quantity;
	
	command[1] = theItem->inventoryLetter;
	confirmMessages();
	
	if ((theItem->flags & ITEM_EQUIPPED) && theItem->quantity <= 1) {
		sprintf(buf, "Are you sure you want to throw your %s?", theName);
		if (!confirm(buf, false)) {
			return;
		}
		if (theItem->flags & ITEM_CURSED) {
			sprintf(buf, "You cannot unequip your %s; it appears to be cursed.", theName);
			messageWithColor(buf, &itemMessageColor, false);
			return;
		}
	}
	
	sprintf(buf, "Throw %s %s where? (<hjklyubn>, mouse, or <tab>)",
			(theItem->quantity > 1 ? "a" : "your"),
			theName);
	temporaryMessage(buf, false);
	maxDistance = (12 + 2 * max(rogue.strength - player.weaknessAmount + rogue.mightBonus
								- (rogue.shield ? rogue.shield->kind * 2 + 1 : 0)
								- (rogue.offhandWeapon ? rogue.offhandWeapon->strengthRequired / 3 : 0) - 12, 2));
	autoTarget = (theItem->category & (WEAPON | POTION)) ? true : false;
	if (chooseTarget(zapTarget, maxDistance, true, autoTarget, false, false)) {
		if ((theItem->flags & ITEM_EQUIPPED) && theItem->quantity <= 1) {
            unequipItem(theItem, false);
        }
		
		command[2] = '\0';
		recordKeystrokeSequence(command);
		recordMouseClick(mapToWindowX(zapTarget[0]), mapToWindowY(zapTarget[1]), true, false);
		
		confirmMessages();
		
		thrownItem = generateItem(ALL_ITEMS, -1);
		*thrownItem = *theItem; // clone the item
		thrownItem->flags &= ~ITEM_EQUIPPED;
		thrownItem->quantity = 1;
		
		itemName(thrownItem, theName, false, false, NULL);
		originLoc[0] = player.xLoc;
		originLoc[1] = player.yLoc;
		
		throwItem(thrownItem, &player, zapTarget, maxDistance);
	} else {
		return;
	}
	
	// Now decrement or delete the thrown item out of the inventory.
	if (theItem->quantity > 1) {
		theItem->quantity--;
	} else {
		removeItemFromChain(theItem, packItems);
		deleteItem(theItem);
	}
	playerTurnEnded();
}

void enchantItem(item *theItem) {
	short loc[2], i;
	creature *monst;
	
	if (theItem->flags & ITEM_CURSED) {
		theItem->flags &= ~ITEM_CURSED;
	}
	switch (theItem->category) {
		case WEAPON:
			theItem->strengthRequired = max(0, theItem->strengthRequired - 1);
			theItem->enchant1++;
			if (theItem->quiverNumber) {
				theItem->quiverNumber = rand_range(1, 60000);
			}
			theItem->hiddenRunicEnchantsRequired--;
			break;
		case ARMOR:
			theItem->strengthRequired = max(0, theItem->strengthRequired - 1);
			theItem->enchant1++;
			theItem->hiddenRunicEnchantsRequired--;
			break;
		case SHIELD:
			theItem->strengthRequired = max(0, theItem->strengthRequired - 1);
			theItem->shieldBlows++;
			theItem->shieldMinBlow++;
			theItem->enchant1++;
			theItem->flags &= ~(ITEM_BROKEN);
			theItem->hiddenRunicEnchantsRequired--;
			break;
		case RING:
			theItem->enchant1++;
			updateRingBonuses();
			if (theItem->kind == RING_CLAIRVOYANCE) {
				updateClairvoyance();
				displayLevel();
			}
			if (theItem->kind == RING_TELEPATHY) {
				updateTelepathy();
				displayLevel();
			}
			break;
		case TALISMAN:
			if (theItem->kind < TALISMAN_MAX_ENCHANT) {
				theItem->enchant1++;
				if (theItem->kind == TALISMAN_MADNESS && player.status[STATUS_HALLUCINATING]) {
					recalculateEquipmentBonuses();
				}
				if (theItem->kind == TALISMAN_FLAMESPIRIT && player.status[STATUS_BURNING]) {
					updateRingBonuses();
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_CLAIRVOYANCE) || (rogue.ringRight && rogue.ringRight->kind == RING_CLAIRVOYANCE)) {
						updateClairvoyance();
						updateClairvoyance(); // Yes, we have to call this twice.
						displayLevel();
					}
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_TELEPATHY) || (rogue.ringRight && rogue.ringRight->kind == RING_TELEPATHY)) {
						updateTelepathy();
						updateTelepathy(); // Yes, we have to call this twice.
						displayLevel();
					}
				}
				if (theItem->kind == TALISMAN_SPIDER) {
					// Place spiders summoned by talisman of spiders
					if (rogue.talisman && rogue.talisman->kind == TALISMAN_SPIDER) {
						
						if (rogue.depthLevel > 2) {
							loc[0] = rogue.downLoc[0];
							loc[1] = rogue.downLoc[1];
						} else {
							loc[0] = rogue.upLoc[0];
							loc[1] = rogue.upLoc[1];
						}
						
						if (getQualifyingLocNear(loc, loc[0], loc[1], false, 0,
												 (T_PATHING_BLOCKER),
												 (HAS_MONSTER | HAS_PLAYER | HAS_STAIRS | IS_IN_MACHINE), false, false)) {
							monst = generateMonster(MK_SPIDER, false);
							monst->xLoc = loc[0];
							monst->yLoc = loc[1];
							monst->spawnDepth = 9; // spider normal minimum depth is 9
							pmap[loc[0]][loc[1]].flags |= (HAS_MONSTER);
							becomeAllyWith(monst);
							for (i = 3; i < rogue.talisman->enchant1; i++) { // to avoid integer overflow
								monst->xpxp += XPXP_NEEDED_FOR_LEVELUP * pow(1.1, (double) rogue.talisman->enchant1 - 3);
								addXPXPToAlly(0, monst); // don't allow spiders to learn anything new to start with
							}
							wakeUp(monst);
						}
					}
				}
			}
			else if (theItem->kind == TALISMAN_BALANCE) { // Talisman of balance allows you to make rings worse.
				talismanTable[TALISMAN_BALANCE].identified = true;
				if (!rogue.ringRight && !rogue.ringLeft) {
					break;
				} else if (!rogue.ringRight) {
					theItem = rogue.ringLeft;
				} else if (!rogue.ringLeft) {
					theItem = rogue.ringRight;
				} else if ((rogue.ringRight->enchant1 > 0) == (rogue.ringLeft->enchant1 > 0)) {
					if (rand_percent(50)) {
						theItem = rogue.ringRight;
					} else {
						theItem = rogue.ringLeft;
					}
				} else if (rogue.ringLeft->enchant1 <= 0) {
					theItem = rogue.ringLeft;
				} else {
					theItem = rogue.ringRight;
				}
				
				theItem->enchant1--;
				updateRingBonuses();
			}
			break;
		case STAFF:
			theItem->enchant1++;
			theItem->charges++;
			theItem->enchant2 = 500 / theItem->enchant1;
			break;
		case WAND:
			//theItem->charges++;
			theItem->charges += wandTable[theItem->kind].range.lowerBound;
			break;
		case CHARM:
			theItem->enchant1++;			
			theItem->charges = min(0, theItem->charges); // Enchanting instantly recharges charms.
			
			//                    theItem->charges = theItem->charges
			//                    * charmRechargeDelay(theItem->kind, theItem->enchant1)
			//                    / charmRechargeDelay(theItem->kind, theItem->enchant1 - 1);
			
			break;
		case POTION:
			elixirTable[theItem->kind].identified = potionTable[theItem->kind].identified;
			theItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
			makeItemInto(theItem, ELIXIR, theItem->kind);
			break;
		case SCROLL:
			tomeTable[theItem->kind].identified = scrollTable[theItem->kind].identified;
			theItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
			makeItemInto(theItem, TOME, theItem->kind);
			break;
		case ELIXIR:
			elixirTable[theItem->kind].identified = potionTable[theItem->kind].identified = true;
			theItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
			makeItemInto(theItem, CHARM, theItem->kind);
			break;
		case TOME:
			tomeTable[theItem->kind].identified = scrollTable[theItem->kind].identified = true;
			theItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
			makeItemInto(theItem, CHARM, theItem->kind + NUMBER_POTION_KINDS); // charms based on scrolls are after charms based on potions
		case FOOD:
			theItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
			if (theItem->kind == RATION) {
				makeItemInto(theItem, POTION, POTION_STRENGTH);
			} else if (theItem->kind == FRUIT) {
				makeItemInto(theItem, POTION, POTION_LIFE);
			}
		default:
			break;
	}
	
	if (theItem->flags & ITEM_EQUIPPED) {
		equipItem(theItem, true);
	}	
}

// Applied a potion of descent to anything other than another potion of descent
void descendItem(item *theItem) {
	char buf[COLS], buf2[COLS];
	short x, y;

	x = theItem->xLoc;
	y = theItem->yLoc;
		
	theItem->flags |= ITEM_PREPLACED;
		
	// Remove from item chain.
	removeItemFromChain(theItem, floorItems);
		
	pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);
		
	// Add to next level's chain.
	theItem->nextItem = levels[rogue.depthLevel-1 + 1].items;
	levels[rogue.depthLevel-1 + 1].items = theItem;
		
	refreshDungeonCell(x, y);
		
	// Well duh...
	potionTable[POTION_DESCENT].identified = true;
	
	itemName(theItem, buf2, false, false, NULL);
	sprintf(buf,"the %s sink%s into the floor and is gone!", buf2, theItem->quantity > 1 ? "" : "s");
	message(buf,false);
}

// This applies a potion to darts, resulting in coated darts of various flavours.
// You need to destroy the potion elsewhere.
void applyPotiontoDarts(item *thePotion, item *theDarts) {	
	// Paranoia
	if (!(theDarts->category & WEAPON) || (theDarts->kind != DART) || !(thePotion->category & POTION)) {
		return;
	}
	
	switch (thePotion->kind)
	{
		case POTION_TELEPATHY:
		case POTION_DETECT_MAGIC:
			theDarts->kind = MARKING_DART;
			if (thePotion->kind == POTION_TELEPATHY && potionTable[POTION_DETECT_MAGIC].identified) {
				potionTable[POTION_TELEPATHY].identified = true;
			} else if (thePotion->kind == POTION_DETECT_MAGIC && potionTable[POTION_TELEPATHY].identified) {
				potionTable[POTION_DETECT_MAGIC].identified = true;
			}
			break;
		case POTION_LEVITATION:
		case POTION_WINDS:
			theDarts->kind = GRAPPLING_DART;
			if (thePotion->kind == POTION_LEVITATION && potionTable[POTION_WINDS].identified) {
				potionTable[POTION_LEVITATION].identified = true;
			} else if (thePotion->kind == POTION_WINDS && potionTable[POTION_LEVITATION].identified) {
				potionTable[POTION_WINDS].identified = true;
			}
			break;
		case POTION_FIRE_IMMUNITY:
		case POTION_WATER:
			theDarts->kind = EXTINGUISHING_DART;
			if (thePotion->kind == POTION_FIRE_IMMUNITY && potionTable[POTION_WATER].identified) {
				potionTable[POTION_FIRE_IMMUNITY].identified = true;
			} else if (thePotion->kind == POTION_WATER && potionTable[POTION_FIRE_IMMUNITY].identified) {
				potionTable[POTION_WATER].identified = true;
			}
			break;
		case POTION_POISON:
		case POTION_LICHEN:
			theDarts->kind = POISON_DART;
			if (thePotion->kind == POTION_POISON && potionTable[POTION_LICHEN].identified) {
				potionTable[POTION_POISON].identified = true;
			} else if (thePotion->kind == POTION_LICHEN && potionTable[POTION_POISON].identified) {
				potionTable[POTION_LICHEN].identified = true;
			}
			break;
		case POTION_HALLUCINATION:
			theDarts->kind = DISCORD_DART;
			potionTable[POTION_HALLUCINATION].identified = true;			
			break;
		case POTION_CONFUSION:
			theDarts->kind = CONFUSING_DART;
			potionTable[POTION_CONFUSION].identified = true;			
			break;
		// Explosive potions will always be 'known'
		case POTION_INCINERATION:
			potionTable[POTION_INCINERATION].identified = true;
			// Fall through
		case POTION_EXPLOSION:
			theDarts->kind = INCENDIARY_DART;
			break;
		case POTION_DARKNESS:
			theDarts->kind = DARKNESS_DART;
			potionTable[POTION_DARKNESS].identified = true;			
			break;
		default:
			if (rand_percent(50)) {
				theDarts->kind = SLOWING_DART;
			} else {
				theDarts->kind = WEAKNESS_DART;
			}
			break;
	}

	// Prevent the item stacking
	theDarts->quiverNumber = rand_range(1, 60000);
	theDarts->damage = weaponTable[theDarts->kind].range;	
}

// This applies a rune to a weapon or armor.
void applyRunetoWeaponorArmor(item *theRune, item *theItem)
{
	// Paranoia
	if (!(theRune->category & KEY) || (theRune->kind < KEY_RUNE_ARMOR) || (theRune->kind > KEY_RUNE_WEAPON) || !(theItem->category & (WEAPON | ARMOR | SHIELD))) {
		return;
	}
	
	// Apply
	theItem->enchant2 = theRune->enchant2;
	theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
	
	// Prevent old darts stacking with new darts
	if ((theItem->category & WEAPON) && theItem->kind >= DART) {
		theItem->quiverNumber = rand_range(1, 60000);
	}

	/* Rune branding makes the item dormant for a while */
	theItem->flags |= (ITEM_NO_PICKUP | ITEM_PLAYER_AVOIDS | ITEM_DORMANT);
	theItem->charges += rand_range(5, 10);
}

// This applies a rune to a wand or staff.
void applyRunetoWandorStaff(item *theRune, item *theItem)
{
	// Paranoia
	if (!(theRune->category & KEY) || (theRune->kind < KEY_RUNE_WAND) || (theRune->kind > KEY_RUNE_STAFF) || !(theItem->category & (WAND | STAFF))) {
		return;
	}

	// Check restrictions
	if (!boltRunicAllowedOnItem(theRune->enchant2, theItem->kind, theItem, (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) != 0)) {
		message("applying the rune has no effect!", false); // shouldn't ever happen unless putting identical runes on one item
		return;
	}
	
	// Apply
	theItem->boltEnchants[theItem->numberOfBoltEnchants++] = theRune->enchant2;
	theItem->flags |= (ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED);
	
	/* Rune branding makes the item dormant for a while */
	theItem->flags |= (ITEM_NO_PICKUP | ITEM_PLAYER_AVOIDS | ITEM_DORMANT);
	theItem->charges += rand_range(5, 10);
}

enum itemEFfects {
	IDENTIFY				= 1,
	DETECT					= 2,
	ENCHANT					= 4,
	IDENTIFY_RUNIC			= 8,
	HINT_RUNIC				= 16,
	KNOW_CHARGES			= 32,
};

void detectMagicOnItem(item *theItem) {
    theItem->flags |= ITEM_MAGIC_DETECTED;
    if ((theItem->category & (WEAPON | ARMOR))
        && theItem->enchant1 == 0
        && !(theItem->flags & ITEM_RUNIC)) {
        
        identify(theItem);
    }
}

boolean affectItem(item *theItem, short itemEffect, int x, int y) {
	boolean hadEffect = false;
	
	if (itemEffect & ENCHANT) {
		enchantItem(theItem);
		hadEffect = true;
	}
	if (itemEffect & IDENTIFY) {
		if (theItem->flags & ITEM_CAN_BE_IDENTIFIED) {
			identify(theItem);
			hadEffect = true;
		} else if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY
				   && (theItem->category & (WEAPON | ARMOR)) && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)) {
			identify(theItem);
			hadEffect = true;
		}
	}
	if (itemEffect & HINT_RUNIC) {
		if (itemRunic(theItem)) {
			hadEffect = true;
			theItem->flags |= (ITEM_RUNIC_HINTED);
		}
	}
	if (itemEffect & IDENTIFY_RUNIC) {
		if (itemRunic(theItem)) {
			hadEffect = true;
			theItem->flags |= (ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED);
		}
	}
	if (itemEffect & KNOW_CHARGES) {
		if (theItem->category & (WAND|STAFF)) {
			hadEffect = true;
			theItem->flags |= (ITEM_MAX_CHARGES_KNOWN);
		}
	}
	if (itemEffect & DETECT) {
		detectMagicOnItem(theItem);
		if (itemMagicChar(theItem)) {
			hadEffect = true;
			if (x != -1 && y != -1) {
				pmap[x][y].flags |= ITEM_DETECTED;
				refreshDungeonCell(x, y);
			}
		}
	}
	return hadEffect;
}

void affectAllItemsOnLevel(unsigned long category, short itemEffect) {
	char buf[COLS*3];
	char *categoryName;
	item *tempItem;
	creature *monst;
	boolean hadEffect = false;
	boolean hadEffect2 = false;
	
	hadEffect = false;
	hadEffect2 = false;
	for (tempItem = floorItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
		if (!category || (tempItem->category & category) != 0) {
			hadEffect |= affectItem(tempItem, itemEffect, tempItem->xLoc, tempItem->yLoc);
		}
	}
	for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
		if (monst->carriedItem) {
			if (!category || (monst->carriedItem->category & category) != 0) {
				hadEffect |= affectItem(monst->carriedItem, itemEffect, monst->xLoc, monst->yLoc);
			}
		}
	}
	for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
		if (!category || (tempItem->category & category) != 0) {
			hadEffect |= affectItem(tempItem, itemEffect, -1, -1);
		}
	}
	switch(category) {
		case FOOD:
			categoryName = "food";
			break;
		case WEAPON:
			if (!hadEffect && !hadEffect2) {
				categoryName = "weapons";
			} else if (itemEffect & IDENTIFY_RUNIC) {
				categoryName = "all runes on weapons";
			} else {
				categoryName = "weapons";
			}
			break;
		case ARMOR:
			if (!hadEffect && !hadEffect2) {
				categoryName = "armor";
			} else if (itemEffect & IDENTIFY_RUNIC) {
				categoryName = "all runes on armor";
			} else {
				categoryName = "armor";
			}
			break;
		case SHIELD:
			if (!hadEffect && !hadEffect2) {
				categoryName = "shields";
			} else if (itemEffect & (HINT_RUNIC | IDENTIFY_RUNIC)) {
				categoryName = "runes on shields";
			} else {
				categoryName = "shields";
			}
			break;
		case POTION:
			categoryName = "potions";
			break;
		case ELIXIR:
			categoryName = "elixirs";
			break;
		case SCROLL:
			categoryName = "scrolls";
			break;
		case TOME:
			categoryName = "tomes";
			break;
		case STAFF:
			if (!hadEffect && !hadEffect2) {
				categoryName = "staffs";				
			} else if (itemEffect & IDENTIFY_RUNIC) {
				categoryName = "all runes on staffs";
			} else if (itemEffect & KNOW_CHARGES) {
				categoryName = "charges on staffs";
			} else {
				categoryName = "staffs";
			}
			break;
		case WAND:
			if (!hadEffect && !hadEffect2) {
				categoryName = "wands";				
			} else if (itemEffect & IDENTIFY_RUNIC) {
				categoryName = "all runes on staffs";
			} else if (itemEffect & KNOW_CHARGES) {
				categoryName = "charges on wands";
			} else {
				categoryName = "wands";
			}
			break;
		case RING:
			categoryName = "rings";
			break;
		case CHARM:
			categoryName = "charms";
			break;
		case TALISMAN:
			categoryName = "talismans";
			break;
		case GOLD:
			categoryName = "gold";
			break;
		case AMULET:
			categoryName = "amulets";
			break;
		case GEM:
			categoryName = "lumenstones";
			break;
		case KEY:
			categoryName = "keys";
			break;
		default:
			categoryName = "magic";
			break;			
	}
	if (itemEffect & DETECT) {
		if (hadEffect || hadEffect2) {
			sprintf(buf, "you can somehow feel the %s of %s ",
					itemEffect & (IDENTIFY | IDENTIFY_RUNIC) ? "identity" : (itemEffect & KNOW_CHARGES ? "number" : "presence"), categoryName);
			if (hadEffect && hadEffect2) {
				strcat(buf, "on the level and in your pack.");
			} else if (hadEffect) {
				strcat(buf, "on the level.");
			} else {
				strcat(buf, "in your pack.");
			}
		} else {
			sprintf(buf, "you can somehow feel the absence of %s%s on the level and in your pack.", categoryName, (itemEffect & IDENTIFY ? " to identify" : ""));
		}
		message(buf, false);
	}
	if (itemEffect & ENCHANT) {
		if (hadEffect || hadEffect2) {
			sprintf(buf, "all the %s ", categoryName);
			if (hadEffect && hadEffect2) {
				strcat(buf, "on the level and in your pack");
			} else if (hadEffect) {
				strcat(buf, "on the level");
			} else {
				strcat(buf, "in your pack");
			}
			strcat(buf, " gleam in the darkness.");
		} else {
			sprintf(buf, "you can somehow feel the absence of %s on the level and in your pack.", categoryName);
		}
		message(buf, false);
	}
}

void identifyAll(item *theItem) {
	short i;
	
	switch (theItem->category) {
		case WEAPON:
			affectAllItemsOnLevel(WEAPON, (DETECT | IDENTIFY | IDENTIFY_RUNIC));
			break;
		case ARMOR:
			affectAllItemsOnLevel(ARMOR, (DETECT | IDENTIFY | IDENTIFY_RUNIC));
			break;
		case SHIELD:
			affectAllItemsOnLevel(SHIELD, (DETECT | IDENTIFY | IDENTIFY_RUNIC));
			break;
		case POTION:
		case ELIXIR:
			for (i = 0; i < NUMBER_POTION_KINDS; i++) {
				potionTable[i].identified = true;
				elixirTable[i].identified = true;
			}
			message("you discover the colors of all potions and chemistry of all elixirs.", false);
			break;
		case SCROLL:
		case TOME:
			for (i = 0; i < NUMBER_SCROLL_KINDS; i++) {
				scrollTable[i].identified = true;
				tomeTable[i].identified = true;
			}
			message("you discover the titles of all tomes and scrolls.", false);
			break;						
		case WAND:
			for (i = 0; i < NUMBER_WAND_KINDS; i++) {
				wandTable[i].identified = true;
			}
			message("you discover which metals all wands are made from.", false);
			break;
		case STAFF:
			for (i = 0; i < NUMBER_STAFF_KINDS; i++) {
				staffTable[i].identified = true;
			}
			message("you discover which woods all staffs are made from.", false);
			break;
		case TALISMAN:
			for (i = 0; i < NUMBER_TALISMAN_KINDS; i++) {
				talismanTable[i].identified = true;
			}
			message("you discover which materials all talismans are made from.", false);
			break;
		case RING:
			for (i = 0; i < NUMBER_RING_KINDS; i++) {
				ringTable[i].identified = true;
			}
			message("you discover which gemstones all rings are made from.", false);
			break;
		default:
			break;
	}
}



// merge two items. order is:
// 1. combine two identical potions or scrolls
// 2a. cause item to descend a level if the other is a potion/elixir of descent
// 2b. enchant one item if the other is a scroll/tome of enchantment
// 2c. duplicate one item if the other is a scroll/tome of duplication
// 3a. apply a potion to darts
// 3b. apply runes to weapons, armor, staffs, wands
// 4. negate one item if the other is a scroll/tome of negation
// 5. convert charm if the other is a potion/scroll
// 6a. explode mismatched potions
// 6b. TODO: read mismatched scrolls
void mergeItems(item *theFirstItem, item *theSecondItem)
{
	if ((theFirstItem->category & POTION) && (theSecondItem->category & POTION) && (theFirstItem->kind == theSecondItem->kind)) {
//		theFirstItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
//		makeItemInto(theFirstItem, ELIXIR, theFirstItem->kind); // first item is always the one to be destroyed
		theSecondItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
		makeItemInto(theSecondItem, ELIXIR, theSecondItem->kind);
	} else if ((theFirstItem->category & SCROLL) && (theSecondItem->category & SCROLL) && (theFirstItem->kind == theSecondItem->kind)) {
		//		theFirstItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
//		makeItemInto(theFirstItem, TOME, theFirstItem->kind); // first item is always the one to be destroyed
		theSecondItem->flags &= ITEM_MAGIC_DETECTED; // clear all flags except magic detection status
		makeItemInto(theSecondItem, TOME, theSecondItem->kind);
	} else if ((theFirstItem->category & (POTION | ELIXIR)) && (theFirstItem->kind == POTION_DESCENT)) {
		descendItem(theSecondItem);
		burnItem(theFirstItem);
	} else if ((theSecondItem->category & (POTION | ELIXIR)) && (theSecondItem->kind == POTION_DESCENT)) {
		descendItem(theFirstItem);
		burnItem(theSecondItem);

/*	} else if ((theFirstItem->category & TOME) && (theFirstItem->kind == TOME_ENCHANTING)) {
		affectAllItemsOnLevel(theSecondItem->category, ENCHANT);
	} else if ((theSecondItem->category & TOME) && (theSecondItem->kind == TOME_ENCHANTING)) {
		affectAllItemsOnLevel(theFirstItem->category, ENCHANT);*/
	} else if ((theFirstItem->category & (SCROLL | TOME)) && (theFirstItem->kind == SCROLL_ENCHANTING)) {
		enchantItem(theSecondItem);
	} else if ((theSecondItem->category & (SCROLL | TOME)) && (theSecondItem->kind == SCROLL_ENCHANTING)) {
		enchantItem(theFirstItem);

	} else if ((theFirstItem->category & (SCROLL | TOME)) && (theFirstItem->kind == SCROLL_DUPLICATION)) {
		duplicateItem(theSecondItem, theSecondItem->category & TOME ? 2 : 1, false);
	} else if ((theSecondItem->category & (SCROLL | TOME)) && (theSecondItem->kind == SCROLL_DUPLICATION)) {
		duplicateItem(theFirstItem, theFirstItem->category & TOME ? 2 : 1, false);
		
	} else if ((theFirstItem->category & POTION) && (theSecondItem->category & WEAPON) && (theSecondItem->kind == DART)) {
		applyPotiontoDarts(theFirstItem, theSecondItem);
	} else if ((theSecondItem->category & POTION) && (theFirstItem->category & WEAPON) && (theFirstItem->kind == DART)) {
		applyPotiontoDarts(theSecondItem, theFirstItem);
	
	} else if ((theFirstItem->category & KEY) && (theFirstItem->kind == KEY_RUNE_ARMOR) && (theSecondItem->category & (ARMOR | SHIELD))) {
		applyRunetoWeaponorArmor(theFirstItem, theSecondItem);
	}
	else if ((theFirstItem->category & KEY) && (theFirstItem->kind == KEY_RUNE_WEAPON) && (theSecondItem->category & (WEAPON))) {
		applyRunetoWeaponorArmor(theFirstItem, theSecondItem);
	}
	else if ((theSecondItem->category & KEY) && (theSecondItem->kind == KEY_RUNE_ARMOR) && (theFirstItem->category & (ARMOR | SHIELD))) {
		applyRunetoWeaponorArmor(theSecondItem, theFirstItem);
	}
	else if ((theSecondItem->category & KEY) && (theSecondItem->kind == KEY_RUNE_WEAPON) && (theFirstItem->category & (WEAPON))) {
		applyRunetoWeaponorArmor(theSecondItem, theFirstItem);
	}
	
	else if ((theFirstItem->category & KEY) && (theFirstItem->kind == KEY_RUNE_WAND) && (theSecondItem->category & (WAND))) {
		applyRunetoWandorStaff(theFirstItem, theSecondItem);
	}
	else if ((theFirstItem->category & KEY) && (theFirstItem->kind == KEY_RUNE_WAND) && (theSecondItem->category & (WAND))) {
		applyRunetoWandorStaff(theFirstItem, theSecondItem);
	}
	else if ((theSecondItem->category & KEY) && (theSecondItem->kind == KEY_RUNE_STAFF) && (theFirstItem->category & (STAFF))) {
		applyRunetoWandorStaff(theSecondItem, theFirstItem);
	}
	else if ((theSecondItem->category & KEY) && (theSecondItem->kind == KEY_RUNE_STAFF) && (theFirstItem->category & (STAFF))) {
		applyRunetoWandorStaff(theSecondItem, theFirstItem);

	} else if ((theFirstItem->category & (SCROLL | TOME)) && (theFirstItem->kind == SCROLL_NEGATION)) {
		negateItem(theSecondItem);
		if (theFirstItem->category & TOME) {
            negationBlast("the tome", true, true, true);
		}
	} else if ((theSecondItem->category & (SCROLL | TOME)) && (theSecondItem->kind == SCROLL_NEGATION)) {
		negateItem(theFirstItem);
		if (theSecondItem->category & TOME) {
            negationBlast("the tome", true, true, true);
		}

	} else if ((theFirstItem->category & (POTION)) && (theSecondItem->category & CHARM)) {
		makeItemInto(theSecondItem, CHARM, theFirstItem->kind);
	} else if ((theSecondItem->category & (POTION)) && (theFirstItem->category & CHARM)) {
		makeItemInto(theFirstItem, CHARM, theSecondItem->kind);
	} else if ((theFirstItem->category & (SCROLL)) && (theSecondItem->category & CHARM)) {
		makeItemInto(theSecondItem, CHARM, theFirstItem->kind + NUMBER_POTION_KINDS);
	} else if ((theSecondItem->category & (SCROLL)) && (theFirstItem->category & CHARM)) {
		makeItemInto(theFirstItem, CHARM, theSecondItem->kind + NUMBER_POTION_KINDS);
		
		// care taken here to avoid referring to scrolls after the explosion in the instance the potion is incineration. note we get away with using descent above.
	} else if ((theFirstItem->category & POTION) && !(theSecondItem->category & SCROLL) && explodeItem(theFirstItem, theSecondItem->xLoc, theSecondItem->yLoc, "flask", "shatters")) {
		if (theSecondItem->category & POTION) {
			explodeItem(theSecondItem, theFirstItem->xLoc, theFirstItem->yLoc, "flask", "shatters");
		}
		burnItem(theFirstItem);
		burnItem(theSecondItem);
	} else if ((theSecondItem->category & POTION) && !(theFirstItem->category & SCROLL) && explodeItem(theSecondItem, theFirstItem->xLoc, theFirstItem->yLoc, "flask", "shatters")) {
		if (theFirstItem->category & POTION) {
			explodeItem(theFirstItem, theSecondItem->xLoc, theSecondItem->yLoc, "flask", "shatters");
		}
		burnItem(theFirstItem);
		burnItem(theSecondItem);
	} else if ((theFirstItem->category & POTION) && explodeItem(theFirstItem, theSecondItem->xLoc, theSecondItem->yLoc, "flask", "shatters")) {
		burnItem(theFirstItem);
	} else if ((theSecondItem->category & POTION) && explodeItem(theSecondItem, theFirstItem->xLoc, theFirstItem->yLoc, "flask", "shatters")) {
		burnItem(theSecondItem);
	} else if ((theFirstItem->category & (SCROLL | TOME)) && (theFirstItem->kind == SCROLL_IDENTIFY)) {
		if (theSecondItem->flags & ITEM_CAN_BE_IDENTIFIED) {
			identify(theSecondItem);
		}
		if (theFirstItem->category & ELIXIR) {
			identifyAll(theSecondItem);
		}
	} else if ((theSecondItem->category & (SCROLL | TOME)) && (theSecondItem->kind == SCROLL_IDENTIFY)) {
		if (theFirstItem->flags & ITEM_CAN_BE_IDENTIFIED) {
			identify(theFirstItem);
		}
		if (theFirstItem->category & TOME) {
			identifyAll(theFirstItem);
		}
	} else if ((theFirstItem->category & (POTION | ELIXIR)) && (theFirstItem->kind == POTION_DETECT_MAGIC)) {
		if (theFirstItem->category & ELIXIR) {
			affectAllItemsOnLevel(STAFF|WAND, KNOW_CHARGES|HINT_RUNIC);
			affectAllItemsOnLevel(WEAPON|ARMOR|SHIELD, IDENTIFY);
		}
		affectAllItemsOnLevel(CAN_BE_DETECTED, DETECT);
	} else if ((theSecondItem->category & (POTION | ELIXIR)) && (theSecondItem->kind == POTION_DETECT_MAGIC)) {
		if (theSecondItem->category & ELIXIR) {
			affectAllItemsOnLevel(STAFF|WAND, KNOW_CHARGES|HINT_RUNIC);
			affectAllItemsOnLevel(WEAPON|ARMOR|SHIELD, IDENTIFY);
		}
		affectAllItemsOnLevel(CAN_BE_DETECTED, DETECT);
	}		
}

boolean useCharm(item *theItem) {
	char buf[COLS], buf2[COLS];
	creature *monst;

	switch (theItem->kind) {
		case CHARM_HEALTH:
			heal(&player, charmHealing(theItem->enchant1));
			message("You feel much healthier.", false);
			break;
		case CHARM_STRENGTH:
			if (player.status[STATUS_WEAKENED]) {
				player.status[STATUS_WEAKENED] = 1;
			}
			player.strengthAmount += 2 * theItem->enchant1;
			player.maxStatus[STATUS_STRONGER] = player.status[STATUS_STRONGER] = max(player.status[STATUS_STRONGER], charmEffectDuration(theItem->kind, theItem->enchant1));
			updateEncumbrance();
			messageWithColor("strength surges through your body.", &advancementMessageColor, false);
			break;
		case CHARM_TELEPATHY:
			makePlayerTelepathic(charmEffectDuration(theItem->kind, theItem->enchant1), false);
			break;
		case CHARM_LEVITATION:
			player.maxStatus[STATUS_LEVITATING] = player.status[STATUS_LEVITATING] = max(player.status[STATUS_LEVITATING], charmEffectDuration(theItem->kind, theItem->enchant1));
			player.bookkeepingFlags &= ~MONST_SEIZED; // break free of holding monsters
			message("you float into the air!", false);
			break;
		case CHARM_DETECT_MAGIC:
			affectAllItemsOnLevel(CAN_BE_DETECTED, DETECT);
			break;
		case CHARM_HASTE:
			haste(&player, charmEffectDuration(theItem->kind, theItem->enchant1), false);
			break;
		case CHARM_FIRE_IMMUNITY:
			player.maxStatus[STATUS_IMMUNE_TO_FIRE] = player.status[STATUS_IMMUNE_TO_FIRE] = max(player.status[STATUS_IMMUNE_TO_FIRE], charmEffectDuration(theItem->kind, theItem->enchant1));
			if (player.status[STATUS_BURNING]) {
				extinguishFireOnCreature(&player);
			}
			message("you no longer fear fire.", false);
			break;
		case CHARM_INVISIBILITY:
			imbueInvisibility(&player, charmEffectDuration(theItem->kind, theItem->enchant1), false);
			message("You shiver as a chill runs up your spine.", false);
			break;
		case CHARM_DISCORD:
			for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
				if ((pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW)
					&& !(monst->info.flags & MONST_INANIMATE)) {
					monst->maxStatus[STATUS_DISCORDANT] = monst->status[STATUS_DISCORDANT] = max(monst->status[STATUS_DISCORDANT], charmEffectDuration(theItem->kind, theItem->enchant1));
				}
			}
			message("the charm emits a hallucinogenic flash of light!", false);
			lightFlash(&rainbow, 0, IN_FIELD_OF_VIEW, 15, DCOLS, player.xLoc, player.yLoc);
			break;
		case CHARM_TELEPORTATION:
			teleport(&player);
			break;
		case CHARM_RECHARGING:
			rechargeItems(STAFF, false);
			break;
		case CHARM_PROTECTION:
		case CHARM_PROTECTION_2:
			player.protectionAmount = max(player.protectionAmount, charmProtection(theItem->enchant1));
			player.status[STATUS_SHIELDED] = min(charmEffectDuration(theItem->kind, theItem->enchant1), player.protectionAmount);
			player.maxStatus[STATUS_SHIELDED] = player.status[STATUS_SHIELDED];
			flashMonster(&player, boltColors[BOLT_SHIELDING], 100);
			message("A shimmering shield coalesces around you.", false);
			break;
		case CHARM_CAUSE_FEAR:
			causeFear("your charm", false, false);
			break;
		case CHARM_NEGATION:
			negationBlast("your charm", false, false, false);
			break;
		case CHARM_SHATTERING:
			messageWithColor("your charm emits a wave of turquoise light that pierces the nearby walls!", &itemMessageColor, false);
			crystalize(player.xLoc, player.yLoc, charmShattering(theItem->enchant1));
			break;
		case CHARM_CHARMING:
			//					identify(theItem);
			//					messageWithColor("this is a charm of charming", &itemMessageColor, true);
			rogue.charming = theItem->enchant1;
			if (!numberOfMatchingPackItems(CHARMING_CHARM, 0, 0, false)) {
				confirmMessages();
				message("you have no lesser charms that can be enchanted.", false);
				return false;
			}
			do {
				theItem = promptForItemOfType(CHARMING_CHARM, 0, 0,
											  "Enchant which lesser charm? (a-z; shift for more info)", false);
				confirmMessages();
				if (theItem && (!(theItem->category & (CHARM)) || theItem->enchant1 >= rogue.charming)) {
						message("Can't enchant that.", true);
				}
				if (rogue.gameHasEnded) {
					return false;
				}
			} while (theItem == NULL || !(theItem->category & (CHARM)) || (theItem->enchant1 >= rogue.charming));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			if (theItem && (theItem->category & (CHARM)) && (theItem->enchant1 < rogue.charming)) {
				enchantItem(theItem);
				itemName(theItem, buf, false, false, NULL);
				sprintf(buf2, "your %s gleam%s in the darkness.", buf, (theItem->quantity == 1 ? "s" : ""));
				messageWithColor(buf2, &itemMessageColor, false);
			} else {
				message("Charms can only enchant charms will less enchants than they have.", false);
			}
			break;
		case CHARM_IDENTIFY:
			//					identify(theItem);
			//					updateIdentifiableItems();
			//					sprintf(buf,"this is a charm of identify.");
			//					messageWithColor(buf, &itemMessageColor, true);
			if (numberOfMatchingPackItems((ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM), ITEM_CAN_BE_IDENTIFIED, 0, false) == 0) {
				message("everything in your pack is already identified.", false);
				return false;
			}
			do {
				theItem = promptForItemOfType((ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM), ITEM_CAN_BE_IDENTIFIED, 0, "Identify what? (a-z; shift for more info)", false);
				if (rogue.gameHasEnded) {
					return false;
				}
				if (theItem && !(theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
					confirmMessages();
					itemName(theItem, buf2, true, true, NULL);
					sprintf(buf, "you already know %s %s.", (theItem->quantity > 1 ? "they're" : "it's"), buf2);
					messageWithColor(buf, &itemMessageColor, false);
				}
			} while (theItem == NULL || !(theItem->category & (ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM)) || !(theItem->flags & ITEM_CAN_BE_IDENTIFIED));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			if (theItem->flags & ITEM_CAN_BE_IDENTIFIED) {
				identify(theItem);
				itemName(theItem, buf, true, true, NULL);
				sprintf(buf2, "%s %s.", (theItem->quantity == 1 ? "this is" : "these are"), buf);
				messageWithColor(buf2, &itemMessageColor, false);
			}
			break;
		case CHARM_MAGIC_MAPPING:
			magicMapping(player.xLoc, player.yLoc, theItem->enchant1 * 5, 0, 0, 0, 0);
			lightFlash(&magicMapFlashColor, 0, MAGIC_MAPPED, 15, theItem->enchant1 * 5, player.xLoc, player.yLoc);
			break;
		case CHARM_DUPLICATION:
//			identify(theItem);
//			messageWithColor("this is a charm of duplication.", &itemMessageColor, true);

			if (!numberOfMatchingPackItems((WEAPON | ARMOR | SHIELD | POTION | DUPLICATE_SCROLL), (ITEM_CAN_BE_IDENTIFIED | ITEM_CURSED), 0, false)) {
				confirmMessages();
				message("you have nothing that can be duplicated.", false);
				return false;
			}
			do {
				theItem = promptForItemOfType((WEAPON | ARMOR | SHIELD | POTION | DUPLICATE_SCROLL), (ITEM_CAN_BE_IDENTIFIED | ITEM_CURSED), 0,
											  "Duplicate what? (a-z; shift for more info)", false);
				confirmMessages();
				if (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | SHIELD | POTION | SCROLL)) ||
					 !((theItem->flags & (ITEM_CAN_BE_IDENTIFIED)) || itemMagicChar(theItem) == BAD_MAGIC_CHAR)) {
					message("Can't duplicate that.", true);
				}
				if (rogue.gameHasEnded) {
					return false;
				}
			} while (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | SHIELD | POTION | SCROLL)) ||
					 !((theItem->flags & (ITEM_CAN_BE_IDENTIFIED)) || itemMagicChar(theItem) == BAD_MAGIC_CHAR));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			theItem->flags |= ITEM_MAGIC_DETECTED;
			if (itemMagicChar(theItem) == BAD_MAGIC_CHAR) {
				duplicateItem(theItem, 0, true);
			}
			if (itemMagicChar(theItem) != BAD_MAGIC_CHAR) {
				message("Charms can only duplicate bad items.", false);
			}
			break;
		case CHARM_AGGRAVATE_MONSTERS:
			aggravateMonsters(100, 0, false, true, false);
			lightFlash(&gray, 0, (DISCOVERED | MAGIC_MAPPED), 10, DCOLS / 2, player.xLoc, player.yLoc);
			message("the charm emits a piercing shriek that echoes throughout the dungeon!", false);
			break;
		case CHARM_SUMMON_MONSTERS:
			summonMonstersAroundPlayer(min(3,theItem->enchant1),min(3, theItem->enchant1), charmEffectDuration(theItem->kind,theItem->enchant1));
			break;
		default:
			drinkPotion(theItem);
			break;
	}
	return true;
}

void useTalisman(item *theItem) {
	short dropLoc[2];
	char buf[COLS], buf2[COLS];
	
	switch (theItem->kind) {
		case TALISMAN_ALCHEMY:
			messageWithColor("you can combine two of the same potions or scrolls.", &itemMessageColor, true);
			if (numberOfMatchingPackItems(COMBINE_2_POTION_OR_SCROLL, 0, 0, false) == 0) {
				message("you must have at least 2 of the same potions or scrolls.", false);
				break;
			}
			theItem = promptForItemOfType((COMBINE_2_POTION_OR_SCROLL), 0, 0,
										  "Combine which 2 potions or scrolls? (a-z; shift for more info)", false);
			if ((theItem->category & (POTION | SCROLL)) && theItem->quantity > 1) {
				recordKeystroke(theItem->inventoryLetter, false, false);
				if (theItem->quantity > 2) {
					theItem->quantity -= 2;
					theItem = generateItem(theItem->category == POTION ? ELIXIR : TOME, theItem->kind);
					if (numberOfItemsInPack() < MAX_PACK_ITEMS || theItem->category & GOLD || itemWillStackWithPack(theItem)) {
						addItemToPack(theItem);
					}
					else if (!itemAtLoc(player.xLoc, player.yLoc)) {
						dropItem(theItem);
						itemName(theItem, buf, false, true, NULL);
						sprintf(buf2, "you drop %s.", buf);
						messageWithColor(buf2, &itemMessageColor, false);
					}
					else {
						getQualifyingLocNear(dropLoc, player.xLoc, player.yLoc, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
						placeItem(theItem, dropLoc[0], dropLoc[1]);
						refreshDungeonCell(dropLoc[0], dropLoc[1]);
						itemName(theItem, buf, false, true, NULL);
						sprintf(buf2, "you drop %s.", buf);
						messageWithColor(buf2, &itemMessageColor, false);
					}					
				} else {
					makeItemInto(theItem, theItem->category == POTION ? ELIXIR : TOME, theItem->kind);
					theItem->quantity = 1;
				}
			}
		default:
			break;
	}
}

void apply(item *theItem, boolean recordCommands) {
	char buf[COLS], buf2[COLS];
	unsigned char command[10];
	short zapTarget[2], originLoc[2], maxDistance, c, i, d;
	boolean autoTarget, targetAllies, passThroughCreatures, commandsRecorded, revealItemType;
	unsigned long categoryMask = (SCROLL|TOME|FOOD|POTION|ELIXIR|STAFF|WAND|CHARM);
	unsigned long boltHasFlags;
	short numberOfBolts;
	float boltPower;
	item *theItem2;
	creature *monst;
	
	commandsRecorded = !recordCommands;
	c = 0;
	command[c++] = APPLY_KEY;
	
	revealItemType = false;
	
	if (rogue.talisman && rogue.talisman->kind == TALISMAN_ASSASSIN) {
		categoryMask |= (COATING_DARTS);
	}
	
	if (rogue.talisman && (/*rogue.talisman->kind == TALISMAN_CHAOS || */rogue.talisman->kind == TALISMAN_SHAPE_CHANGING || rogue.talisman->kind == TALISMAN_ALCHEMY)) {
		categoryMask |= (APPLY_TALISMAN);
	}

	if (!theItem) {
		theItem = promptForItemOfType((categoryMask), 0, 0,
									  "Apply what? (a-z, shift for more info; or <esc> to cancel)", true);
	}
	
	if (theItem == NULL) {
		return;
	}
	command[c++] = theItem->inventoryLetter;
	confirmMessages();
	switch (theItem->category) {
		case FOOD:
			if (STOMACH_SIZE - player.status[STATUS_NUTRITION] < foodTable[theItem->kind].strengthRequired) { // Not hungry enough.
				sprintf(buf, "You're not hungry enough to fully enjoy the %s. Eat it anyway?",
						(theItem->kind == RATION ? "food" : "mango"));
				if (!confirm(buf, false)) {
					return;
				}
			}
			player.status[STATUS_NUTRITION] = min(foodTable[theItem->kind].strengthRequired + player.status[STATUS_NUTRITION], STOMACH_SIZE);
			if (theItem->kind == RATION) {
				messageWithColor("That food tasted delicious!", &itemMessageColor, false);
			} else {
				messageWithColor("My, what a yummy mango!", &itemMessageColor, false);
			}
			break;
		case POTION:
			command[c] = '\0';
			recordKeystrokeSequence(command);
			commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
			if (!potionTable[theItem->kind].identified) {
				revealItemType = true;
			}
			drinkPotion(theItem);
			break;
		case ELIXIR:
			command[c] = '\0';
			recordKeystrokeSequence(command);
			commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
			if (!elixirTable[theItem->kind].identified) {
				revealItemType = true;
			}
			drinkPotion(theItem);
			break;
		case SCROLL:
			command[c] = '\0';
			recordKeystrokeSequence(command);
			commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
			if (!scrollTable[theItem->kind].identified
				&& theItem->kind != SCROLL_ENCHANTING
				&& theItem->kind != SCROLL_IDENTIFY
				&& theItem->kind != SCROLL_DUPLICATION) {
				
				revealItemType = true;
			}
			readScroll(theItem);
			break;
		case TOME:
			command[c] = '\0';
			recordKeystrokeSequence(command);
			commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
			if (!tomeTable[theItem->kind].identified
				&& theItem->kind != TOME_ENCHANTING
				&& theItem->kind != TOME_IDENTIFY
				&& theItem->kind != TOME_DUPLICATION
				&& theItem->kind != TOME_TELEPORT) {
				
				revealItemType = true;
			}
			readScroll(theItem);
			break;
		case STAFF:
		case WAND:
			if (theItem->charges <= 0 && (theItem->flags & ITEM_IDENTIFIED)) {
				itemName(theItem, buf2, false, false, NULL);
				sprintf(buf, "Your %s has no charges.", buf2);
				messageWithColor(buf, &itemMessageColor, false);
				return;
			}
			// collect bolt information
			boltHasFlags = 0;
			numberOfBolts = 1;
			boltPower = 1.0;
			for (i = ((rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY) ? 0 : 1); i <= theItem->numberOfBoltEnchants; i++) {
				boltHasFlags |= boltRunicCatalog[theItem->boltEnchants[i]].flags;
				numberOfBolts += boltRunicCatalog[theItem->boltEnchants[i]].number;
				boltPower *= boltRunicCatalog[theItem->boltEnchants[i]].power;				
			}
			if (theItem->category & STAFF) {
				boltHasFlags |= BOLT_FROM_STAFF;
			} else {
				boltHasFlags |= BOLT_FROM_WAND;
			}
			if (!staffTable[theItem->kind].identified && !wandTable[theItem->kind].identified) {
				boltHasFlags |= (BOLT_HIDE_DETAILS);
			}			
			temporaryMessage("Direction? (<hjklyubn>, mouse, or <tab>; <return> to confirm)", false);
			itemName(theItem, buf2, false, false, NULL);
			sprintf(buf, "Zapping your %s:", buf2);
			printString(buf, mapToWindowX(0), 1, &itemMessageColor, &black, NULL);
			if ((!staffTable[theItem->kind].identified || !wandTable[theItem->kind].identified || theItem->kind != BOLT_BECKONING)
					 && (boltHasFlags & BOLT_DISTANCE)
					 && (theItem->flags & (ITEM_RUNIC_IDENTIFIED))) {
				maxDistance = (short) (theItem->kind & WAND ? 10 : theItem->enchant1) * boltPower + 2;
			} else if ((staffTable[theItem->kind].identified || wandTable[theItem->kind].identified) && theItem->kind == BOLT_BLINKING
				&& (theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN))) {
				maxDistance = (short)  (theItem->kind & WAND ? 10 : theItem->enchant1) * boltPower * 2 + 2;
			} else if (staffTable[theItem->kind].identified && theItem->category == STAFF
					   && (theItem->kind == STAFF_BECKONING || theItem->kind == STAFF_TELEPORT || theItem->kind == STAFF_MIRRORED_TOTEM)
					   && (theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN))) {
				maxDistance = (short)  (theItem->kind & WAND ? 10 : theItem->enchant1) * boltPower * 2 + 2;
			} else {
				maxDistance = -1;
			}
			autoTarget = true;
			if ((staffTable[theItem->kind].identified || wandTable[theItem->kind].identified) &&
				(theItem->kind == BOLT_BLINKING
				 || theItem->kind == BOLT_TUNNELING
				 || theItem->kind == BOLT_NATURE
				 || theItem->kind == BOLT_SENTRY
				 || theItem->kind >= BOLT_UNDERWORM)) {
					autoTarget = false;
				}
			targetAllies = false;
			if ((staffTable[theItem->kind].identified || wandTable[theItem->kind].identified) &&
				 (theItem->kind == BOLT_HEALING || theItem->kind == BOLT_HASTE || theItem->kind == BOLT_SHIELDING || theItem->kind == BOLT_REFLECTION
				  || theItem->kind == BOLT_INVISIBILITY || theItem->kind == BOLT_PLENTY)) {
					targetAllies = true;
				}
			passThroughCreatures = false;
			if ((staffTable[theItem->kind].identified || wandTable[theItem->kind].identified) &&
				(((theItem->flags & ITEM_RUNIC_IDENTIFIED) && (boltHasFlags & (BOLT_SELECTIVE | BOLT_PENETRATING | BOLT_BOUNCES | BOLT_X_RAY))) ||
				 (theItem->kind == BOLT_BLINKING && (theItem->flags & ITEM_RUNIC_IDENTIFIED) && (boltHasFlags & BOLT_DISTANCE)) ||
				 theItem->kind == BOLT_LIGHTNING || theItem->kind == BOLT_FIRE || theItem->kind == BOLT_FORCE || theItem->kind == BOLT_SENTRY || theItem->kind == BOLT_UNDERWORM)) {
				passThroughCreatures = true;
			}
			if (numberOfBolts < 1) {
				numberOfBolts = 1;		// this could result in destroying 0 charge staffs if we ever reduce the number of bolts otherwise
			}
			for (i = 0; i < numberOfBolts; i++) {
				if (i == 0 || (theItem->kind == BOLT_BLINKING && !(boltHasFlags & BOLT_CHAINING))
					|| (theItem->kind != BOLT_BLINKING && theItem->kind != BOLT_TOAD && !(boltHasFlags & (BOLT_CHAINING | BOLT_BOUNCES)))) {
					if (chooseTarget(zapTarget, maxDistance, false, autoTarget, targetAllies, passThroughCreatures)) {
						command[c] = '\0';
						if (i || !commandsRecorded) {
							recordKeystrokeSequence(command);
							recordMouseClick(mapToWindowX(zapTarget[0]), mapToWindowY(zapTarget[1]), true, false);
							commandsRecorded = true;
							c = 0;
						}
						confirmMessages();
					} else {
						if (i) {
							recordKeystroke(ESCAPE_KEY, false, false);
							break;
						} else {
							return;
						}
					}
				}
				if (i > 0 && (boltHasFlags & BOLT_CHAINING)) {
					originLoc[0] = zapTarget[0];
					originLoc[1] = zapTarget[1];
					d = COLS * COLS;
					if ((pmap[originLoc[0]][originLoc[1]].flags & IN_FIELD_OF_VIEW) && rand_percent(33)) {
						zapTarget[0] = player.xLoc;
						zapTarget[1] = player.yLoc;
						d = (player.xLoc - originLoc[0]) * (player.xLoc - originLoc[0]) + (player.yLoc - originLoc[1]) * (player.xLoc - originLoc[1]) + 1; // hack -- if any monster equally close, player will not be target
					}
					for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
						if ((monst->bookkeepingFlags & MONST_BOLT_CHAINED) || (monst->xLoc == originLoc[0] && monst->yLoc == originLoc[1])) {
							continue;
						}
						if ((monst->xLoc - originLoc[0]) * (monst->xLoc - originLoc[0]) + (monst->yLoc - originLoc[1]) * (monst->xLoc - originLoc[1]) > d) {
							continue;
						}
						if (theItem->kind == BOLT_TUNNELING || theItem->kind == BOLT_UNDERWORM || (boltHasFlags & BOLT_X_RAY) || openPathBetween(monst->xLoc, monst->yLoc, originLoc[0], originLoc[1])) {
							d = (monst->xLoc - originLoc[0]) * (monst->xLoc - originLoc[0]) + (monst->yLoc - originLoc[1]) * (monst->xLoc - originLoc[1]);
							zapTarget[0] = monst->xLoc;
							zapTarget[1] = monst->yLoc;
						}
					}
					if (originLoc[0] == zapTarget[0] && originLoc[1] == zapTarget[1]) {
						break;
					}					
				} else {
					originLoc[0] = player.xLoc;
					originLoc[1] = player.yLoc;
				}
				if (theItem->charges > 0) {
					if (zap(originLoc, zapTarget,
							theItem->kind,		// bolt type
							(theItem->category == STAFF ? theItem->enchant1 : 10), boltPower, // bolt level, power
							boltHasFlags)) {
						if (theItem->category & STAFF) {
							if (!staffTable[theItem->kind].identified) {
								staffTable[theItem->kind].identified = true;
								revealItemType = true;
							}
						} else {
							if (!wandTable[theItem->kind].identified) {
								wandTable[theItem->kind].identified = true;
								revealItemType = true;
							}
						}
						
						if (i > 0 || theItem->numberOfBoltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
							theItem->flags |= (ITEM_RUNIC_IDENTIFIED);
						}
					}
					if (boltHasFlags & BOLT_X_RAY) {
						theItem->flags |= (ITEM_RUNIC_IDENTIFIED);
					}
				} else {
					itemName(theItem, buf2, false, false, NULL);
					if (theItem->category == STAFF) {
						sprintf(buf, "Your %s fizzles; it must be out of charges for now.", buf2);
						theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
					} else {
						sprintf(buf, "Your %s fizzles; it must be depleted.", buf2);
						theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
					}
					messageWithColor(buf, &itemMessageColor, false);
					playerTurnEnded();
					return;
				}
			}
			// Shield and offhand weapons makes wands and staffs twice as slow
			if (rogue.shield || rogue.offhandWeapon || (boltHasFlags & BOLT_SLOWLY)) {
				player.ticksUntilTurn += 2 * player.movementSpeed;
			} else if (boltHasFlags & BOLT_QUICKLY) {
				player.ticksUntilTurn += player.movementSpeed / 2;
			}
			for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
				monst->bookkeepingFlags &= ~MONST_BOLT_CHAINED;
			}
			player.bookkeepingFlags &= ~MONST_BOLT_CHAINED;
			break;
        case CHARM:
			if ((theItem->charges > 0) && !(theItem->flags & ITEM_OVERCHARGED)) {
				itemName(theItem, buf2, false, false, NULL);
				sprintf(buf, "Your %s hasn't finished recharging.", buf2);
				messageWithColor(buf, &itemMessageColor, false);
				return;
			}
			if (theItem->charges > 0) {
				theItem->flags &= ~ITEM_OVERCHARGED;
			}
			command[c] = '\0';
			recordKeystrokeSequence(command);
			commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
			if (!useCharm(theItem)) { // either didn't have any valid items or quit the game
				return; // avoid discharging charm in either instance
			}
            break;
		case TALISMAN:
			if (rogue.talisman && theItem == rogue.talisman && (/*theItem->kind == TALISMAN_CHAOS ||*/ theItem->kind == TALISMAN_SHAPE_CHANGING)) {
				break;
			} else if (rogue.talisman && theItem == rogue.talisman && theItem->kind == TALISMAN_ALCHEMY) {
				command[c] = '\0';
				recordKeystrokeSequence(command);
				commandsRecorded = true; // have to record as further keystrokes are necessary
				useTalisman(theItem);
				break;
			}
			// Fall through
		case WEAPON:
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_ASSASSIN && theItem->kind == DART) {
				command[c] = '\0';
				recordKeystrokeSequence(command);
				commandsRecorded = true; // have to record as further keystrokes are necessary
				messageWithColor("you can coat your darts with a potion.", &itemMessageColor, true);
				if (numberOfMatchingPackItems(POTION, 0, 0, false) == 0) {
					message("you have no potions.", false);
					break;
				}
				theItem2 = promptForItemOfType((POTION), 0, 0,
												"Coat your darts with what? (a-z; shift for more info)", false);
				if (rogue.gameHasEnded) {
					return;
				}
				if (theItem2 && (theItem2->category & POTION)) {
					recordKeystroke(theItem2->inventoryLetter, false, false);
					confirmMessages();
					applyPotiontoDarts(theItem2, theItem);
					itemName(theItem, buf, true, true, NULL);
					sprintf(buf2, "you have created %s.", buf);
					messageWithColor(buf2, &itemMessageColor, false);
					theItem = theItem2;
				} else if (theItem2) {
					confirmMessages();
					messageWithColor("you must choose a potion.", &itemMessageColor, false);
					return;
				}
				break;
			}
			// Fall through
		default:
			itemName(theItem, buf2, false, true, NULL);
			sprintf(buf, "you can't apply %s.", buf2);
			message(buf, false);
			return;
	}
	
	if (!commandsRecorded) { // to make sure we didn't already record the keystrokes above with staff/wand targeting
		command[c] = '\0';
		recordKeystrokeSequence(command);
	}
	
	// Reveal the item type if appropriate.
	if (revealItemType) {
		autoIdentify(theItem);
	}
	
	if (theItem->category & TALISMAN) {
			// Do nothing.
	} else if (rogue.talisman && rogue.talisman->kind == TALISMAN_SHAPE_CHANGING && (theItem->category & (STAFF | WAND)) &&
			   (((player.info.flags & MA_CAST_HEAL) && theItem->kind == BOLT_HEALING ) ||
				((player.info.flags & MA_CAST_HASTE) && theItem->kind == BOLT_HASTE ) ||
				((player.info.flags & MA_CAST_PROTECTION) && theItem->kind == BOLT_SHIELDING ) ||
				((player.info.flags & MA_CAST_BLINK) && theItem->kind == BOLT_BLINKING ) ||
				((player.info.flags & MA_CAST_NEGATION) && theItem->kind == BOLT_NEGATION ) ||
				((player.info.flags & MA_CAST_SPARK) && theItem->kind == BOLT_LIGHTNING ) ||
				((player.info.flags & MA_CAST_FIRE) && theItem->kind == BOLT_FIRE ) ||
				((player.info.flags & MA_CAST_SLOW) && theItem->kind == BOLT_SLOW ) ||
				((player.info.flags & MA_CAST_DISCORD) && theItem->kind == BOLT_DISCORD ) ||
				((player.info.flags & MA_CAST_BECKONING) && theItem->kind == BOLT_BECKONING ) ||
				((player.info.flags & MA_CAST_SENTRY) && theItem->kind == BOLT_SENTRY ) ||				
				((player.info.flags & MA_CAST_SUMMON) && theItem->kind == BOLT_CONJURATION && player.info.monsterID == MK_GOBLIN_CONJURER ))) {
		// Do nothing.
	} else if (theItem->category & CHARM) {
        theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
    } else if (theItem->charges > 0) {
		theItem->charges--;
		if (theItem->category == STAFF && (theItem->flags & ITEM_OVERCHARGED) && theItem->charges <= theItem->enchant1) { // clear overcharged flag
			theItem->flags &= ~ITEM_OVERCHARGED;
		}
		if (theItem->category == WAND) {
			theItem->enchant2++; // keeps track of how many times the wand has been discharged for the player's convenience
		}
	} else if (theItem->quantity > 1) {
		theItem->quantity--;
	} else {
		removeItemFromChain(theItem, packItems);
		deleteItem(theItem);
	}
	playerTurnEnded();
}

void identify(item *theItem) {
	itemTable *theTable;
	short tableCount, i, lastItem;
	
	tableCount = 0;
	lastItem = -1;
	
	theItem->flags |= ITEM_IDENTIFIED;
	theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
	if (itemRunic(theItem)) {
		theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
	}
	
	switch (theItem->category) {
		case SCROLL:
			scrollTable[theItem->kind].identified = true;
			theTable = scrollTable;
			tableCount = NUMBER_SCROLL_KINDS_SHOWN;
			break;
		case TOME:
			tomeTable[theItem->kind].identified = true;
			break;
		case POTION:
			potionTable[theItem->kind].identified = true;
			theTable = potionTable;
			tableCount = NUMBER_POTION_KINDS_SHOWN;
			break;
		case ELIXIR:
			elixirTable[theItem->kind].identified = true;
			break;
		case WAND:
			wandTable[theItem->kind].identified = true;
			theTable = wandTable;
			tableCount = NUMBER_WAND_KINDS_SHOWN;
			if (theItem->boltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
				theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
			}
			break;
		case STAFF:
			staffTable[theItem->kind].identified = true;
			theTable = staffTable;
			tableCount = NUMBER_STAFF_KINDS_SHOWN;
			if (theItem->boltEnchants || (rogue.talisman && rogue.talisman->kind == TALISMAN_WIZARDRY)) {
				theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
			}
			break;
		case RING:
			ringTable[theItem->kind].identified = true;
			theTable = ringTable;
			tableCount = NUMBER_RING_KINDS_SHOWN;
			break;
		case TALISMAN:
			talismanTable[theItem->kind].identified = true;
			theTable = talismanTable;
			tableCount = NUMBER_TALISMAN_KINDS_SHOWN;
			break;
		case WEAPON:
		case ARMOR:
		case SHIELD:
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_RUNE_MASTERY) {
				theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
			}
			break;
		default:
			break;
	}
	
	if ((!tableCount || theItem->category & TALISMAN || theItem->category & RING) && !(theItem->flags & ITEM_CURSED)) {
		theItem->flags |= ITEM_KNOWN_NOT_CURSED;
	}
	
	if (tableCount) {
		theTable[theItem->kind].identified = true;
		// TODO: this part doesn't work for some reason.
		for (i=0; i<tableCount; i++) {
			if (!(theTable[i].identified)) {
				if (lastItem != -1) {
					return; // at least two unidentified items remain
				}
				lastItem = i;
			}
		}
		if (lastItem != -1 && (!(theItem->category & STAFF) || lastItem <= STAFF_SENTRY)) {
			// exactly one unidentified item remains
			theTable[lastItem].identified = true;
		}
	}
}

enum monsterTypes chooseVorpalEnemy() {
	short i, index, possCount = 0, deepestLevel = 0, deepestHorde, chosenHorde, failsafe = 25;
	enum monsterTypes candidate;
	
	do {
		for (i=0; i<NUMBER_HORDES; i++) {
			if (hordeCatalog[i].minLevel >= rogue.depthLevel && !hordeCatalog[i].flags) {
				possCount += hordeCatalog[i].frequency;
			}
			if (hordeCatalog[i].minLevel > deepestLevel) {
				deepestHorde = i;
				deepestLevel = hordeCatalog[i].minLevel;
			}
		}
		
		if (possCount == 0) {
			chosenHorde = deepestHorde;
		} else {
			index = rand_range(1, possCount);
			for (i=0; i<NUMBER_HORDES; i++) {
				if (hordeCatalog[i].minLevel >= rogue.depthLevel && !hordeCatalog[i].flags) {
					if (index <= hordeCatalog[i].frequency) {
						chosenHorde = i;
						break;
					}
					index -= hordeCatalog[i].frequency;
				}
			}
		}
		
		index = rand_range(-1, hordeCatalog[chosenHorde].numberOfMemberTypes - 1);
		if (index == -1) {
			candidate = hordeCatalog[chosenHorde].leaderType;
		} else {
			candidate = hordeCatalog[chosenHorde].memberType[index];
		}
		
		// Handle phylacteries
		if ((monsterCatalog[candidate].flags & MONST_NEVER_VORPAL_ENEMY) && (monsterCatalog[candidate].abilityFlags & MA_ENTER_SUMMONS)) {
			chosenHorde = pickHordeType(0, candidate, 0, 0);

			index = rand_range(0, hordeCatalog[chosenHorde].numberOfMemberTypes - 1);
			candidate = hordeCatalog[chosenHorde].memberType[index];
		}
		
	} while (((monsterCatalog[candidate].flags & MONST_NEVER_VORPAL_ENEMY)
              || (monsterCatalog[candidate].abilityFlags & MA_NEVER_VORPAL_ENEMY))
             && --failsafe > 0);
	return monsterCatalog[candidate].slayID;
}

void updateIdentifiableItem(item *theItem) {
	if (((theItem->category & SCROLL) && scrollTable[theItem->kind].identified == true)
		|| ((theItem->category & POTION) && potionTable[theItem->kind].identified == true)
		|| ((theItem->category & TALISMAN) && theItem->kind > TALISMAN_BALANCE && talismanTable[theItem->kind].identified == true)) {
		
		theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
	} else if ((theItem->category & (RING | STAFF | WAND | TALISMAN))
			   && (theItem->flags & ITEM_IDENTIFIED)
			   && tableForItemCategory(theItem->category)[theItem->kind].identified) {
		
		theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
	} else if ((theItem->category & (WEAPON | ARMOR | SHIELD))
			   && (theItem->flags & ITEM_IDENTIFIED)
			   && (!(theItem->flags & ITEM_RUNIC) || (theItem->flags & ITEM_RUNIC_IDENTIFIED))) {
		
		theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
	}
}

void updateIdentifiableItems() {
	item *theItem;
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		updateIdentifiableItem(theItem);
	}
	for (theItem = floorItems; theItem != NULL; theItem = theItem->nextItem) {
		updateIdentifiableItem(theItem);
	}
}

// also for tomes
void readScroll(item *theItem) {
	short i, j, level;
	item *tempItem;
	boolean hadEffect = false;
	boolean lumenstone;
	boolean tome = (theItem->category & TOME) != 0;
	char buf[2*COLS], buf2[COLS];
	short dropLoc[2];
	unsigned long categories;
	
	switch (theItem->kind) {
		case SCROLL_IDENTIFY:
			identify(theItem);
			updateIdentifiableItems();
			sprintf(buf,"this is a %s of identify.", tome ? "tome" : "scroll");
			messageWithColor(buf, &itemMessageColor, true);
			if (numberOfMatchingPackItems((ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM), tome ? 0 : ITEM_CAN_BE_IDENTIFIED, 0, false) == 0) {
				message("everything in your pack is already identified.", false);
				break;
			}
			do {
				sprintf(buf,"Identify what%s? (a-z; shift for more info)", tome ? " type of item" : "");				
				theItem = promptForItemOfType((ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM), tome ? 0 : ITEM_CAN_BE_IDENTIFIED, 0, buf, false);
				if (rogue.gameHasEnded) {
					return;
				}
				if (!tome && theItem && !(theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
					confirmMessages();
					itemName(theItem, buf2, true, true, NULL);
					sprintf(buf, "you already know %s %s.", (theItem->quantity > 1 ? "they're" : "it's"), buf2);
					messageWithColor(buf, &itemMessageColor, false);
				}
			} while (theItem == NULL || !(theItem->category & (ALL_ITEMS) & ~(CHARM|FOOD|AMULET|GEM)) || !(tome ? true : (theItem->flags & ITEM_CAN_BE_IDENTIFIED)));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			if (theItem->flags & ITEM_CAN_BE_IDENTIFIED) {
				identify(theItem);
				itemName(theItem, buf, true, true, NULL);
				sprintf(buf2, "%s %s.", (theItem->quantity == 1 ? "this is" : "these are"), buf);
				messageWithColor(buf2, &itemMessageColor, false);
			}
			if (tome) {
				identifyAll(theItem);
			}
			break;
		case SCROLL_TELEPORT:
			if (tome) {
				temporaryMessage("Destination? (<hjklyubn>, mouse, or <tab>; <return> to confirm)", false);
				printString("Teleport to where:", mapToWindowX(0), 1, &itemMessageColor, &black, NULL);
				if (chooseTarget(dropLoc, 2*COLS, false, false, false, true)) {
					recordMouseClick(mapToWindowX(dropLoc[0]), mapToWindowY(dropLoc[1]), true, false);
					confirmMessages();
					if (!(pmap[player.xLoc][player.yLoc].flags & (HAS_PLAYER | HAS_MONSTER)) && !cellHasTerrainFlag(dropLoc[0], dropLoc[1], T_OBSTRUCTS_PASSABILITY)) {
						pmap[player.xLoc][player.yLoc].flags &= ~HAS_PLAYER;
						refreshDungeonCell(player.xLoc, player.yLoc);
						player.xLoc = dropLoc[0];
						player.yLoc = dropLoc[1];
						pmap[dropLoc[0]][dropLoc[1]].flags |= HAS_PLAYER;
						refreshDungeonCell(dropLoc[0], dropLoc[1]);
						updateVision(true);
						return;
					}
				}
			}
			teleport(&player);
			break;
		case SCROLL_ENCHANTING:
			identify(theItem);
			sprintf(buf,"this is a %s of enchanting.", tome ? "tome" : "scroll");
			messageWithColor(buf, &itemMessageColor, true);
			categories = WEAPON | ARMOR | SHIELD | RING | STAFF | WAND | CHARM | ENCHANT_TALISMAN;
			lumenstone = false;
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_ALCHEMY && !(theItem->category & TOME)) {
				categories |= ENCHANT_POTION | SCROLL;
				for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
					if (theItem->category & GEM) {
						lumenstone = true;
						break;
					}
				}
			}
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_WITCHCRAFT && !(theItem->category & TOME)) {
				categories |= TOME | ELIXIR;
			}
			if (!numberOfMatchingPackItems(categories, 0, 0, false)) {
				confirmMessages();
				message("you have nothing that can be enchanted.", false);
				break;
			}
			do {
				theItem = promptForItemOfType(categories, 0, 0,
											  "Enchant what? (a-z; shift for more info)", false);
				confirmMessages();
				if (theItem == NULL || !(theItem->category & (categories | POTION | TALISMAN))
					|| ((theItem->category & POTION) && (!(categories & ENCHANT_POTION) || (theItem->kind == POTION_LIFE && !lumenstone)))
					|| ((theItem->category & TALISMAN) && (theItem->kind > TALISMAN_MAX_ENCHANT) && (theItem->kind != TALISMAN_BALANCE))) {
					if ((categories & ENCHANT_POTION) && (theItem->category & POTION) && theItem->kind == POTION_LIFE) {
						message("You need a lumenstone.", true);
					} else {
						message("Can't enchant that.", true);
					}
				}
				if (rogue.gameHasEnded) {
					return;
				}
			} while (theItem == NULL || !(theItem->category & (categories | POTION | TALISMAN))
					 || ((theItem->category & POTION) && (!(categories & ENCHANT_POTION) || (theItem->kind == POTION_LIFE && !lumenstone)))
					 || ((theItem->category & TALISMAN) && (theItem->kind > TALISMAN_MAX_ENCHANT) && (theItem->kind != TALISMAN_BALANCE)));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			if (theItem->flags & ITEM_CURSED) {
				sprintf(buf2, "a malevolent force leaves your %s.", buf);
				messageWithColor(buf2, &itemMessageColor, false);
			}			
			if (tome) {
				affectAllItemsOnLevel(theItem->category, ENCHANT);
			} else if (theItem) {
				if ((theItem->category & (POTION | SCROLL | TOME | ELIXIR | FOOD)) && theItem->quantity > 1) {
					theItem->quantity--;
					theItem = generateItem(theItem->category, theItem->kind);
					enchantItem(theItem);
					if (numberOfItemsInPack() < MAX_PACK_ITEMS || theItem->category & GOLD || itemWillStackWithPack(theItem)) {
						addItemToPack(theItem);
					}
					else if (!itemAtLoc(player.xLoc, player.yLoc)) {
						dropItem(theItem);
						itemName(theItem, buf, false, true, NULL);
						sprintf(buf2, "you drop %s.", buf);
						messageWithColor(buf2, &itemMessageColor, false);
					}
					else {
						getQualifyingLocNear(dropLoc, player.xLoc, player.yLoc, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
						placeItem(theItem, dropLoc[0], dropLoc[1]);
						refreshDungeonCell(dropLoc[0], dropLoc[1]);
						itemName(theItem, buf, false, true, NULL);
						sprintf(buf2, "you drop %s.", buf);
						messageWithColor(buf2, &itemMessageColor, false);
					}
				} else if (theItem) {
					enchantItem(theItem);
				}
				itemName(theItem, buf, false, false, NULL);
				sprintf(buf2, "your %s gleam%s in the darkness.", buf, (theItem->quantity == 1 ? "s" : ""));
				messageWithColor(buf2, &itemMessageColor, false);
			}
			break;
		case SCROLL_RECHARGING:
			rechargeItems(STAFF|WAND|CHARM, tome);
			break;
		case SCROLL_PROTECT_ARMOR:
			if (tome && rogue.armor && !(rogue.armor->flags & ITEM_RUNIC) && (rogue.armor->hiddenRunicEnchantsRequired > 0)) {
				tempItem = rogue.armor;
				rogue.armor->flags |= (ITEM_RUNIC | ITEM_IDENTIFIED);
				rogue.armor->enchant2 = A_IMMUNITY;
				// vorpal enemy generated when the object originally generated
			} else if (tome && rogue.shield && !(rogue.shield->flags & ITEM_RUNIC) && (rogue.armor->hiddenRunicEnchantsRequired > 0)) {
				tempItem = rogue.shield;
				rogue.shield->flags |= (ITEM_RUNIC | ITEM_IDENTIFIED);
				rogue.shield->enchant2 = A_IMMUNITY;				
				// vorpal enemy generated when the object originally generated
			} else if (rogue.armor && !(rogue.armor->flags & ITEM_PROTECTED)) {
				tempItem = rogue.armor;
			} else if (rogue.shield && !(rogue.shield->flags & ITEM_PROTECTED)) {
				tempItem = rogue.shield;
			} else {
				message("a protective golden light surrounds you, but it quickly disperses.", false);
				tempItem = NULL;
			}
			if (tempItem) {
				tempItem->flags |= ITEM_PROTECTED;
				itemName(tempItem, buf2, false, false, NULL);
				sprintf(buf, "a protective golden light covers your %s.", buf2);
				messageWithColor(buf, &itemMessageColor, false);
				if (tempItem->flags & ITEM_CURSED) {
					sprintf(buf, "a malevolent force leaves your %s.", buf2);
					messageWithColor(buf, &itemMessageColor, false);
					tempItem->flags &= ~ITEM_CURSED;
				}
			}			
			break;
		case SCROLL_PROTECT_WEAPON:
			if (tome && rogue.weapon && !(rogue.weapon->flags & ITEM_RUNIC) && (rogue.weapon->hiddenRunicEnchantsRequired > 0)) {
				tempItem = rogue.weapon;
				rogue.weapon->flags |= (ITEM_RUNIC | ITEM_IDENTIFIED);
				rogue.weapon->enchant2 = W_SLAYING;
				// vorpal enemy generated when the object originally generated
			} else if (tome && rogue.offhandWeapon && !(rogue.offhandWeapon->flags & ITEM_RUNIC) && (rogue.offhandWeapon->hiddenRunicEnchantsRequired > 0)) {
				tempItem = rogue.offhandWeapon;
				rogue.offhandWeapon->flags |= (ITEM_RUNIC | ITEM_IDENTIFIED);
				rogue.offhandWeapon->enchant2 = W_SLAYING;				
				// vorpal enemy generated when the object originally generated
			} else if (rogue.weapon && !(rogue.weapon->flags & ITEM_PROTECTED)) {
				tempItem = rogue.weapon;
			} else if (rogue.offhandWeapon && !(rogue.weapon->flags & ITEM_PROTECTED)) {
				tempItem = rogue.offhandWeapon;
			} else {
				message("a protective golden light covers your empty hands, but it quickly disperses.", false);
				tempItem = NULL;
			}
			if (tempItem) {
				tempItem->flags |= ITEM_PROTECTED;
				itemName(tempItem, buf2, false, false, NULL);
				sprintf(buf, "a protective golden light covers your %s.", buf2);
				messageWithColor(buf, &itemMessageColor, false);
				if (tempItem->flags & ITEM_CURSED) {
					sprintf(buf, "a malevolent force leaves your %s.", buf2);
					messageWithColor(buf, &itemMessageColor, false);
					tempItem->flags &= ~ITEM_CURSED;
				}
                if (tempItem->quiverNumber) {
                    tempItem->quiverNumber = rand_range(1, 60000);
                }
			}
			break;
		case SCROLL_MAGIC_MAPPING:
			confirmMessages();
			if (tome) {
				messageWithColor("this tome has a map of the dungeon in it!", &itemMessageColor, false);
				for (level = 0; level < rogue.depthLevel-1; level++) {
					for (i=0; i<DCOLS; i++) {
						for (j=0; j<DROWS; j++) {
							if (!(levels[level].mapStorage[i][j].flags & DISCOVERED) && levels[level].mapStorage[i][j].layers[DUNGEON] != GRANITE) {
								levels[level].mapStorage[i][j].flags |= MAGIC_MAPPED;
							}
						}
					}
				}
			} else {
				messageWithColor("this scroll has a map on it!", &itemMessageColor, false);
			}
			magicMapping(player.xLoc, player.yLoc, 0, 0, 0, 0, 0);
			lightFlash(&magicMapFlashColor, 0, MAGIC_MAPPED, 15, DCOLS, player.xLoc, player.yLoc);
			break;
		case SCROLL_AGGRAVATE_MONSTER:
			if (tome) {
				aggravateMonsters(0, 10, true, true, true); // drives enemies permanently aggravated and temporarily confused anywhere on level
			} else {
				aggravateMonsters(100, 0, false, true, false); // drives monsters temporarily mad in line of sight
				aggravateMonsters(0, 0, true, false, false); // wakes up everything out of line of sight				
			}
			lightFlash(&gray, 0, (DISCOVERED | MAGIC_MAPPED), 10, DCOLS / 2, player.xLoc, player.yLoc);
			sprintf(buf, "the %s emits a piercing shriek that echoes throughout the dungeon!", tome ? "tome" : "scroll");
			message(buf, false);
			break;
		case SCROLL_SUMMON_MONSTER:
			summonMonstersAroundPlayer(tome ? 1 : 0, 3, 1000);
			break;
		case SCROLL_CAUSE_FEAR:
			causeFear(tome ? "the tome" : "the scroll", tome, tome);
			break;
		case SCROLL_NEGATION:
			negationBlast(tome ? "the tome" : "the scroll", tome, tome, tome);
			for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
				if (tempItem->flags & ITEM_CURSED) {
					hadEffect = true;
					tempItem->flags &= ~ITEM_CURSED;
				}
				if (tome && itemMagicChar(tempItem) == BAD_MAGIC_CHAR) {
					negateItem(tempItem);
				}
				if (tempItem->category & (WEAPON | ARMOR | SHIELD | RING)) {
					tempItem->flags |= (ITEM_KNOWN_NOT_CURSED);
				}
			}
			if (hadEffect) {
				message("your pack glows with a cleansing light, and a malevolent energy disperses.", false);
			}
			hadEffect = true;
			break;
		case SCROLL_SHATTERING:
			sprintf(buf,"the %s emits a wave of turquoise light that pierces the nearby walls!", tome ? "tome" : "scroll");
			messageWithColor(buf, &itemMessageColor, true);
			crystalize(player.xLoc, player.yLoc, tome ? 18 : 9);
			break;
		case SCROLL_DUPLICATION:
			identify(theItem);
			sprintf(buf,"this is a %s of %s.", tome ? "tome" : "scroll", tome ? "triplication" : "duplication");
			messageWithColor(buf, &itemMessageColor, true);
			if (!numberOfMatchingPackItems((ALL_ITEMS | DUPLICATE_SCROLL | DUPLICATE_TOME) & ~(AMULET | GEM | SCROLL | TOME | CHARM), 0, 0, false)) {
				confirmMessages();
				message("you have nothing that can be duplicated.", false);
				break;
			}
			do {
				theItem = promptForItemOfType(((ALL_ITEMS | DUPLICATE_SCROLL | DUPLICATE_TOME) & ~(AMULET | GEM | SCROLL | TOME | CHARM)), 0, 0,
											  "Duplicate what? (a-z; shift for more info)", false);
				confirmMessages();
				if (theItem == NULL || !(theItem->category & (ALL_ITEMS & ~(AMULET | GEM)))
					|| ((theItem->category & (SCROLL | TOME)) && theItem->kind == SCROLL_DUPLICATION)
					|| ((theItem->category & CHARM) && theItem->kind == CHARM_DUPLICATION)) {
					message("Can't duplicate that.", true);
				}
				if (rogue.gameHasEnded) {
					return;
				}
			} while (theItem == NULL || !(theItem->category & (ALL_ITEMS & ~(AMULET | GEM)))
					 || ((theItem->category & (SCROLL | TOME)) && theItem->kind == SCROLL_DUPLICATION)
					 || ((theItem->category & CHARM) && theItem->kind == CHARM_DUPLICATION));
			recordKeystroke(theItem->inventoryLetter, false, false);
			confirmMessages();
			if (theItem) {
				duplicateItem(theItem, tome ? 2 : 1, true);
			}
			break;
	}
}

// also used for elixirs and applying harmful charms
void drinkPotion(item *theItem) {
    char buf[1000];
	enum potionKind newPotion;
	boolean elixir = (theItem->category & ELIXIR) != 0;
	short i, j, maxVolume = 0;
	short **grid;
	short dropLoc[2];
	short gasTypes = WIND-PLAIN_FIRE+1;
	short gasCount[WIND-PLAIN_FIRE+1];
	
	switch (theItem->kind) {
		case POTION_LIFE:
			if (player.status[STATUS_HALLUCINATING] > 1 && rogue.talisman && rogue.talisman->kind == TALISMAN_MADNESS) {
				player.status[STATUS_HALLUCINATING] = max(1, player.status[STATUS_HALLUCINATING]/5);
			} else if (player.status[STATUS_HALLUCINATING] > 1) {
				player.status[STATUS_HALLUCINATING] = 1;
			}
			if (player.status[STATUS_CONFUSED] > 1) {
				player.status[STATUS_CONFUSED] = 1;
			}
			if (player.status[STATUS_NAUSEOUS] > 1) {
				player.status[STATUS_NAUSEOUS] = 1;
			}
			if (player.status[STATUS_SLOWED] > 1) {
				player.status[STATUS_SLOWED] = 1;
			}
			if (player.status[STATUS_WEAKENED] > 1) {
                player.weaknessAmount = 0;
				player.status[STATUS_WEAKENED] = 1;
			}
			if (player.status[STATUS_POISONED] > 1 && rogue.talisman && rogue.talisman->kind == TALISMAN_SPIDER) {
				player.status[STATUS_POISONED] = max(1, player.status[STATUS_POISONED]/5);
			} else if (player.status[STATUS_POISONED]) {
				player.status[STATUS_POISONED] = 0;
			}
			if (player.status[STATUS_DARKNESS] > 0) {
				player.status[STATUS_DARKNESS] = 0;
				updateMinersLightRadius();
				updateVision(true);
			}
			if (player.status[STATUS_LIFESPAN_REMAINING] > 0) {
				player.status[STATUS_LIFESPAN_REMAINING] = 0;
			}
            sprintf(buf, "%syour maximum health increases by %i%%.",
                    ((player.currentHP < player.info.maxHP) ? "you heal completely and " : ""),
                    (player.info.maxHP + 10 + (elixir ? 10 : 0)) * 100 / player.info.maxHP - 100);
            
            player.info.maxHP += 10;
            player.currentHP = player.info.maxHP;
			if (elixir) {
				player.protectionAmount += 1000;
				player.maxStatus[STATUS_SHIELDED] = player.status[STATUS_SHIELDED] = 20;
			}
            updatePlayerRegenerationDelay();
            messageWithColor(buf, &advancementMessageColor, false);
			break;
		case POTION_HALLUCINATION:
			player.status[STATUS_HALLUCINATING] = player.maxStatus[STATUS_HALLUCINATING] = 300;
			message("colors are everywhere! The walls are singing!", false);
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_MADNESS) {
				recalculateEquipmentBonuses();
			}
			break;
		case POTION_INCINERATION:
			if (elixir) {
				player.status[STATUS_IMMUNE_TO_FIRE] += 30;
				player.maxStatus[STATUS_IMMUNE_TO_FIRE] = player.status[STATUS_IMMUNE_TO_FIRE];
				if (player.status[STATUS_BURNING] && (!rogue.talisman || rogue.talisman->kind != TALISMAN_FLAMESPIRIT)) {
					extinguishFireOnCreature(&player);
				}				
			}
			lightFlash(&darkOrange, 0, IN_FIELD_OF_VIEW, 4, 4, player.xLoc, player.yLoc);
			sprintf(buf, "as you %s, it explodes in flame!", theItem->category & CHARM ? "activate the charm" : "uncork the flask");
			message(buf, false);
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_FLAMETHROWER], true, false);
			exposeCreatureToFire(&player);
			break;
		case POTION_DARKNESS:
			if (theItem->category & (POTION | ELIXIR)) {
				player.status[elixir ? STATUS_LIGHTENED : STATUS_DARKNESS] = max(400, player.status[elixir ? STATUS_LIGHTENED : STATUS_DARKNESS]);
				player.maxStatus[elixir ? STATUS_LIGHTENED : STATUS_DARKNESS] = player.status[elixir ? STATUS_LIGHTENED : STATUS_DARKNESS];
				player.status[elixir ? STATUS_DARKNESS : STATUS_LIGHTENED] = player.status[elixir ? STATUS_DARKNESS : STATUS_LIGHTENED] = 0;
				updateMinersLightRadius();
				updateVision(true);
			}
			if (theItem->category & POTION) {
				message("your vision flickers as a cloak of darkness settles around you!", false);
			} else if (theItem->category & ELIXIR) {
				message("your vision improves as the light intensifies around you!", false);					
			} else {
				message("you activate the charm and the lights in the area start fading.", false);
			}
			if (theItem->category & (POTION | CHARM)) {
				spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_DARKNESS_POTION], true, false);
			}			
			break;
		case POTION_DESCENT:
			lightFlash(&darkBlue, 0, IN_FIELD_OF_VIEW, 3, 3, player.xLoc, player.yLoc);
			sprintf(buf, "vapor pours out of the %s and causes the floor to disappear!", theItem->category & CHARM ? "charm" : "flask");
			message(buf, false);
			if (elixir) {
				player.status[STATUS_LEVITATING] += 25;
				player.maxStatus[STATUS_LEVITATING] = player.status[STATUS_LEVITATING];
				player.bookkeepingFlags &= ~MONST_SEIZED; // break free of holding monsters
				message("you float into the air!", false);				
			}
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
			break;
		case POTION_STRENGTH:
			rogue.strength++;
			if (player.status[STATUS_WEAKENED]) {
				player.status[STATUS_WEAKENED] = 1;
			}
			if (elixir) {
				rogue.strength++;
				player.strengthAmount += 6;
				player.maxStatus[STATUS_STRONGER] = player.status[STATUS_STRONGER] = max(player.status[STATUS_STRONGER], 200);
			}
			updateEncumbrance();
			messageWithColor("newfound strength surges through your body.", &advancementMessageColor, false);
			break;
		case POTION_POISON:
			if (elixir) {
				player.status[STATUS_POISON_IMMUNITY] += 50;
				player.maxStatus[STATUS_POISON_IMMUNITY] = player.maxStatus[STATUS_POISON_IMMUNITY];
				player.status[STATUS_POISONED] = player.maxStatus[STATUS_POISONED] = 0;
				player.status[STATUS_NAUSEOUS] = player.maxStatus[STATUS_NAUSEOUS] = 0;
			}						
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
			sprintf(buf, "poisonous gas billows out of the %s!", theItem->category & CHARM ? "charm" : "open flask"); 
			message(buf, false);
			break;
		case POTION_PARALYSIS:
			if (elixir) {
				player.status[STATUS_PARALYSIS_IMMUNITY] += 10;
				player.maxStatus[STATUS_PARALYSIS_IMMUNITY] = player.status[STATUS_PARALYSIS_IMMUNITY];				
			}
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
			sprintf(buf, "your muscles stiffen as a cloud of pink gas bursts from the %s!", theItem->category & CHARM ? "charm" : "open flask"); 
			message(buf, false);
			break;
		case POTION_TELEPATHY:
			makePlayerTelepathic(300, elixir);
			break;
		case POTION_LEVITATION:
			player.maxStatus[STATUS_LEVITATING] = player.status[STATUS_LEVITATING] = max(player.status[STATUS_LEVITATING], 100);
			if (elixir) {
				player.maxStatus[STATUS_LEVITATING_NOT_NEGATABLE] = player.status[STATUS_LEVITATING_NOT_NEGATABLE] = max(player.status[STATUS_LEVITATING_NOT_NEGATABLE], 100);
			}
			player.bookkeepingFlags &= ~MONST_SEIZED; // break free of holding monsters
			message("you float into the air!", false);
			break;
		case POTION_CONFUSION:
			if (elixir) {
				player.status[STATUS_CONFUSION_IMMUNITY] += 50;
				player.maxStatus[STATUS_CONFUSION_IMMUNITY] = player.status[STATUS_CONFUSION_IMMUNITY];				
				player.status[STATUS_CONFUSED] = player.maxStatus[STATUS_CONFUSED] = 0;
			}
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
			sprintf(buf, "a shimmering cloud of rainbow-colored gas billows out of the %s!", theItem->category & CHARM ? "charm" : "open flask"); 
			message(buf, false);
			break;
		case POTION_LICHEN:
			if (elixir) {
				player.status[STATUS_POISON_IMMUNITY] += 50;
				player.maxStatus[STATUS_POISON_IMMUNITY] = player.status[STATUS_POISON_IMMUNITY];
				player.status[STATUS_POISONED] = player.maxStatus[STATUS_POISONED] = 0;
				player.status[STATUS_NAUSEOUS] = player.maxStatus[STATUS_NAUSEOUS] = 0;
			}
			sprintf(buf, "a handful of tiny spores burst out of the %s!", theItem->category & CHARM ? "charm" : "open flask"); 
			message(buf, false);
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
			break;
		case POTION_DETECT_MAGIC:
			if (elixir) {
				affectAllItemsOnLevel(STAFF|WAND, KNOW_CHARGES|HINT_RUNIC);
				affectAllItemsOnLevel(WEAPON|ARMOR|SHIELD, IDENTIFY);
			}
			affectAllItemsOnLevel(CAN_BE_DETECTED, DETECT);
			break;
		case POTION_HASTE_SELF:
			haste(&player, 25, elixir);
			break;
		case POTION_FIRE_IMMUNITY:
			player.maxStatus[STATUS_IMMUNE_TO_FIRE] = player.status[STATUS_IMMUNE_TO_FIRE] = max(player.status[STATUS_IMMUNE_TO_FIRE], 150);
			if (elixir) {
				player.status[STATUS_EXPLOSION_IMMUNITY] = max(player.status[STATUS_EXPLOSION_IMMUNITY], 150);
				player.maxStatus[STATUS_IMMUNE_TO_FIRE_NOT_NEGATABLE] = player.status[STATUS_IMMUNE_TO_FIRE_NOT_NEGATABLE] = max(player.status[STATUS_IMMUNE_TO_FIRE_NOT_NEGATABLE], 150);
			}
			//player.info.flags |= MONST_IMMUNE_TO_FIRE;
			if (player.status[STATUS_BURNING] && (!rogue.talisman || rogue.talisman->kind != TALISMAN_FLAMESPIRIT)) {
				extinguishFireOnCreature(&player);
			}
			message("a comforting breeze envelops you, and you no longer fear fire.", false);
			break;
		case POTION_INVISIBILITY:
			player.maxStatus[STATUS_INVISIBLE] = player.status[STATUS_INVISIBLE] = max(player.status[STATUS_INVISIBLE], 75);
			if (elixir) {
				player.maxStatus[STATUS_IMPROVED_INVISIBLE] = player.status[STATUS_IMPROVED_INVISIBLE] = max(player.status[STATUS_IMPROVED_INVISIBLE], 75);
			}
			message("you shiver as a chill runs up your spine.", false);
			break;
		case POTION_WATER:
			if (elixir) {
				player.maxStatus[STATUS_BLESSED] = player.status[STATUS_BLESSED] = max(player.status[STATUS_BLESSED], 50);
			}			
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_DEEP_WATER_POTION], true, false);
			sprintf(buf, "the %s surges out of your grip as water floods the area!", theItem->category & CHARM ? "charm" : "open flask");
			message(buf, false);
			if (theItem->category & CHARM) {
				getQualifyingLocNear(dropLoc, player.xLoc, player.yLoc, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
				placeItem(theItem, dropLoc[0], dropLoc[1]);
			}
			break;
		case POTION_WINDS:
			if (theItem->category & (POTION | ELIXIR)) {
				newPotion = POTION_WINDS;
				
				for (i = 0; i < gasTypes; i++) {
					gasCount[i] = 0;
				}
				
				grid = allocDynamicGrid();
				fillDynamicGrid(grid, 0);
				
				calculateDistances(grid, player.xLoc, player.yLoc, T_OBSTRUCTS_GAS, NULL, false, false, true);
				
				for (i=0; i<DCOLS; i++) {
					for (j=0; j<DROWS; j++) {
						if (grid[i][j] >= 0 && grid[i][j] < 6) {
							if (pmap[i][j].layers[GAS] >= POISON_GAS && pmap[i][j].layers[GAS] <= WIND) {
								gasCount[pmap[i][j].layers[GAS] - PLAIN_FIRE] += (pmap[i][j].volume + grid[i][j]) / (grid[i][j] + 1); // Even 1 gas always contributes
							} else if (pmap[i][j].layers[GAS] >= PLAIN_FIRE && pmap[i][j].layers[GAS] < POISON_GAS) {
								gasCount[0] += (pmap[i][j].volume + grid[i][j]) / (grid[i][j] + 1);
								pmap[i][j].volume /= grid[i][j] + 1;
							}
						}
					}
				}
				
				freeDynamicGrid(grid);
				
				for (i = 0; i < gasTypes; i++) {
					if (gasCount[i] > maxVolume && gasCount[i] >= 20) {
						maxVolume = gasCount[i];
						switch (i + PLAIN_FIRE) {
							case PLAIN_FIRE:
								newPotion = POTION_INCINERATION;
								break;
							case POISON_GAS:
								newPotion = POTION_POISON;
								break;						
							case CONFUSION_GAS:
								newPotion = POTION_CONFUSION;
								break;
							case ROT_GAS:
								newPotion = POTION_STENCH;
								break;
							case PARALYSIS_GAS:
								newPotion = POTION_PARALYSIS;
								break;
							case METHANE_GAS:
								newPotion = POTION_EXPLOSION;
								break;
							case STEAM:
								newPotion = POTION_WATER;
								break;
							default:
								newPotion = POTION_WINDS;
								break;
						}
					}
				}
			}				
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_WINDS_POTION], true, false);
			sprintf(buf, "strong winds swirl out from the %s!", theItem->category & CHARM ? "charm" : "open flask");
			message(buf, false);
			if (theItem->category & (POTION | ELIXIR)) {				
				theItem = generateItem(elixir ? ELIXIR : POTION, newPotion);
				theItem->flags |= (ITEM_NO_PICKUP | ITEM_PLAYER_AVOIDS | ITEM_DORMANT | ITEM_KIND_AUTO_ID);
				theItem->charges += rand_range(10, 15);
				getQualifyingLocNear(dropLoc, player.xLoc, player.yLoc, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
				placeItem(theItem, dropLoc[0], dropLoc[1]);
				pmap[dropLoc[0]][dropLoc[1]].flags &= ~ITEM_DETECTED;
			}
			break;
		case POTION_STENCH:
			if (elixir) {
				player.status[STATUS_POISON_IMMUNITY] += 50;
				player.maxStatus[STATUS_POISON_IMMUNITY] = player.status[STATUS_POISON_IMMUNITY];
				player.status[STATUS_POISONED] = player.maxStatus[STATUS_POISONED] = 0;
				player.status[STATUS_NAUSEOUS] = player.maxStatus[STATUS_NAUSEOUS] = 0;
			}			
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_ROT_GAS_CLOUD_POTION], true, false);
			sprintf(buf, "you drop the %s in disgust as the air is filled with a hideous stench!", theItem->category & CHARM ? "charm" : "open flask");
			message(buf, false);
			break;
		case POTION_EXPLOSION:
			spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_METHANE_GAS_CLOUD_POTION], true, false);
			sprintf(buf, "an explosive gas escapes from the %s!", theItem->category & CHARM ? "charm" : "open flask");
			message(buf, false);
			break;			
		default:
			message("you feel very strange, as though your body doesn't know how to react!", true);
	}
}

// Used for the Discoveries screen. Returns a number: 1 == good, -1 == bad, 0 == could go either way.
short magicCharDiscoverySuffix(unsigned long category, short kind) {
	short result = 0;
	
	switch (category) {
		case SCROLL:
			switch (kind) {
				case SCROLL_AGGRAVATE_MONSTER:
				case SCROLL_SUMMON_MONSTER:
					result = -1;
					break;
				default:
					result = 1;
					break;
			}
			break;
		case TOME:
		case ELIXIR:
			result = 1;
			break;
		case POTION:
			switch (kind) {
				case POTION_HALLUCINATION:
				case POTION_INCINERATION:
				case POTION_DESCENT:
				case POTION_POISON:
				case POTION_PARALYSIS:
				case POTION_CONFUSION:
				case POTION_LICHEN:
				case POTION_DARKNESS:
				case POTION_WATER:
				case POTION_STENCH:
				case POTION_EXPLOSION:
					result = -1;
					break;
				default:
					result = 1;
					break;
			}
			break;
		case WAND:
		case STAFF:
			if (kind >= BOLT_POLYMORPH) {
				result = -1;
			} else {
				result = 1;
			}
			break;
		case SUMMONING_STAFF:
			result = -1;
			break;
		case RING:
			result = 0;
			break;
		case CHARM:
			result = 1;
			break;
		case TALISMAN:
			if (kind >= /*TALISMAN_TRAP_MASTERY*/ TALISMAN_SACRIFICE) {
				return 1;
			} else {
				return 0;
			}
			break;
	}
	return result;
}

uchar itemMagicChar(item *theItem) {
	switch (theItem->category) {
		case WEAPON:
		case ARMOR:
		case SHIELD:
			if ((theItem->flags & ITEM_CURSED) || theItem->enchant1 < 0) {
				return BAD_MAGIC_CHAR;
			} else if (theItem->enchant1 > 0) {
				return GOOD_MAGIC_CHAR;
			}
			return 0;
			break;
		case SCROLL:
			switch (theItem->kind) {
				case SCROLL_AGGRAVATE_MONSTER:
				case SCROLL_SUMMON_MONSTER:
					return BAD_MAGIC_CHAR;
				default:
					return GOOD_MAGIC_CHAR;
			}
		case TOME:
			return GOOD_MAGIC_CHAR;
		case POTION:
		case ELIXIR:
			switch (theItem->kind) {
				case POTION_HALLUCINATION:
				case POTION_INCINERATION:
				case POTION_DESCENT:
				case POTION_POISON:
				case POTION_PARALYSIS:
				case POTION_CONFUSION:
				case POTION_LICHEN:
				case POTION_DARKNESS:
				case POTION_WATER:
				case POTION_STENCH:
				case POTION_EXPLOSION:
					return BAD_MAGIC_CHAR;
				default:
					return GOOD_MAGIC_CHAR;
			}
		case WAND:
			if (theItem->charges == 0) {
				return 0;
			}
		case STAFF:
			if (theItem->kind >= BOLT_POLYMORPH) {
				return BAD_MAGIC_CHAR;
			} else {
				return GOOD_MAGIC_CHAR;
			}
		case RING:
			if (theItem->flags & ITEM_CURSED || theItem->enchant1 < 0) {
				return BAD_MAGIC_CHAR;
			} else if (theItem->enchant1 > 0) {
				return GOOD_MAGIC_CHAR;
			} else {
				return 0;
			}
		case CHARM:
			return GOOD_MAGIC_CHAR;
		case TALISMAN:
			if (theItem->kind >= TALISMAN_BALANCE) {
				return GOOD_MAGIC_CHAR;
			} else if (theItem->flags & ITEM_CURSED || theItem->enchant1 < 0) {
				return BAD_MAGIC_CHAR;
			} else if (theItem->enchant1 > 0) {
				return GOOD_MAGIC_CHAR;
			} else {
				return 0;
			}
		case AMULET:
			return AMULET_CHAR;
		case KEY:
			if (theItem->flags & ITEM_KEY_DETECTED)
				return KEY_CHAR;
	}
	return 0;
}

void unequip(item *theItem) {
	char buf[COLS], buf2[COLS];
	unsigned char command[3];
	
	command[0] = UNEQUIP_KEY;
	if (theItem == NULL) {
		theItem = promptForItemOfType(ALL_ITEMS, ITEM_EQUIPPED, 0,
									  "Remove (unequip) what? (a-z or <esc> to cancel)", true);
	}
	if (theItem == NULL) {
		return;
	}
	
	command[1] = theItem->inventoryLetter;
	command[2] = '\0';
	
	if (!(theItem->flags & ITEM_EQUIPPED)) {
		itemName(theItem, buf2, false, false, NULL);
		sprintf(buf, "your %s was not equipped.", buf2);
		confirmMessages();
		messageWithColor(buf, &itemMessageColor, false);
		return;
	} else if (rogue.talisman == theItem && theItem->kind == TALISMAN_ALCHEMY && numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS) {
		sprintf(buf, "you can't; you must drop %i duplicate potion%s or scroll%s.",
				numberOfItemsInPackWithoutAlchemy() - MAX_PACK_ITEMS,
				numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS + 1 ? "s" : "",
				numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS + 1 ? "s" : "");
		confirmMessages();
		messageWithColor(buf, &itemMessageColor, false);
		return;
	} else if (theItem->flags & ITEM_CURSED) { // this is where the item gets unequipped
		itemName(theItem, buf2, false, false, NULL);
		sprintf(buf, "you can't; your %s appears to be cursed.", buf2);
		confirmMessages();
		messageWithColor(buf, &itemMessageColor, false);
		return;
	} else {
		recordKeystrokeSequence(command);
		unequipItem(theItem, false);
		if (theItem->category & RING) {
			updateRingBonuses();
		}
		itemName(theItem, buf2, true, true, NULL);
		if (strLenWithoutEscapes(buf2) > 52) {
			itemName(theItem, buf2, false, true, NULL);
		}
		confirmMessages();
		updateEncumbrance();
		sprintf(buf, "you are no longer %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
		messageWithColor(buf, &itemMessageColor, false);
	}
	playerTurnEnded();
}

boolean canDrop() {
	item *theItem;
	
	if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_ITEMS)) {
		return false;
	}
	
	theItem = itemAtLoc(player.xLoc, player.yLoc);
	if (theItem && (theItem->flags & ITEM_NO_PICKUP)) {
		return false;
	}
	return true;
}

void drop(item *theItem) {
	char buf[COLS], buf2[COLS];
	unsigned char command[3];
	
	short tempQuantity;
	
	command[0] = DROP_KEY;
	if (theItem == NULL) {
		theItem = promptForItemOfType(ALL_ITEMS, 0, 0,
									  "Drop what? (a-z, shift for more info; or <esc> to cancel)", true);
	}
	if (theItem == NULL) {
		return;
	}
	command[1] = theItem->inventoryLetter;
	command[2] = '\0';
	
	if ((theItem->flags & ITEM_EQUIPPED) && (theItem->flags & ITEM_CURSED)) {
		itemName(theItem, buf2, false, false, NULL);
		sprintf(buf, "you can't; your %s appears to be cursed.", buf2);
		confirmMessages();
		messageWithColor(buf, &itemMessageColor, false);
	} else if (canDrop()) {
		recordKeystrokeSequence(command);
		if (theItem->flags & ITEM_EQUIPPED) {
			unequipItem(theItem, false);
		}
		// We put the drop message before the item dropping so that the message order is "You dropped an item." "The item burns up.".
		tempQuantity = theItem->quantity;
		if (!(theItem->category & WEAPON)) {
			theItem->quantity = 1;
		}
		itemName(theItem, buf2, true, true, NULL);
		sprintf(buf, "You dropped %s.", buf2);
		theItem->quantity = tempQuantity;
		messageWithColor(buf, &itemMessageColor, false);
		theItem = dropItem(theItem); // This is where it gets dropped.
		theItem->flags |= ITEM_PLAYER_AVOIDS; // Try not to pick up stuff you've already dropped.
		playerTurnEnded();
	} else {
		confirmMessages();
		message("There is already something there.", false);
	}
}

item *promptForItemOfType(unsigned long category,
						  unsigned long requiredFlags,
						  unsigned long forbiddenFlags,
						  char *prompt,
						  boolean allowInventoryActions) {
	char keystroke;
	item *theItem;
	
	if (!numberOfMatchingPackItems(ALL_ITEMS, requiredFlags, forbiddenFlags, true)) {
		return NULL;
	}
	
	temporaryMessage(prompt, false);
	
	keystroke = displayInventory(category, requiredFlags, forbiddenFlags, false, allowInventoryActions);
	
	if (!keystroke) {
		// This can happen if the player does an action with an item directly from the inventory screen via a button.
		return NULL;
	}
	
	if (keystroke < 'a' || keystroke > 'z') {
		confirmMessages();
		if (keystroke != ESCAPE_KEY && keystroke != ACKNOWLEDGE_KEY) {
			message("Invalid entry.", false);
		}
		return NULL;
	}
	
	theItem = itemOfPackLetter(keystroke);
	if (theItem == NULL) {
		confirmMessages();
		message("No such item.", false);
		return NULL;
	}
	
	return theItem;
}

item *itemOfPackLetter(char letter) {
	item *theItem;
	for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
		if (theItem->inventoryLetter == letter) {
			return theItem;
		}
	}
	return NULL;
}

item *itemAtLoc(short x, short y) {
	item *theItem;
	
	if (!(pmap[x][y].flags & HAS_ITEM)) {
		return NULL; // easy optimization
	}
	for (theItem = floorItems->nextItem; theItem != NULL && (theItem->xLoc != x || theItem->yLoc != y); theItem = theItem->nextItem);
	if (theItem == NULL) {
		pmap[x][y].flags &= ~HAS_ITEM;
		hiliteCell(x, y, &white, 75, true);
		rogue.automationActive = false;
		message("ERROR: An item was supposed to be here, but I couldn't find it.", true);
		refreshDungeonCell(x, y);
	}
	return theItem;
}

item *dropItem(item *theItem) {
	item *itemFromTopOfStack, *itemOnFloor;
	
	if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_ITEMS)) {
		return NULL;
	}
	
	itemOnFloor = itemAtLoc(player.xLoc, player.yLoc);
	if (itemOnFloor && (itemOnFloor->flags & ITEM_NO_PICKUP)) {
		return NULL;
	}
	
	if (theItem->quantity > 1 && !(theItem->category & WEAPON)) { // peel off the top item and drop it
		itemFromTopOfStack = generateItem(ALL_ITEMS, -1);
		*itemFromTopOfStack = *theItem; // clone the item
		theItem->quantity--;
		itemFromTopOfStack->quantity = 1;
		if (itemOnFloor) {
			itemOnFloor->inventoryLetter = theItem->inventoryLetter; // just in case all letters are taken
			pickUpItemAt(player.xLoc, player.yLoc);
		}
		placeItem(itemFromTopOfStack, player.xLoc, player.yLoc);
		return itemFromTopOfStack;
	} else { // drop the entire item
		removeItemFromChain(theItem, packItems);
		if (itemOnFloor) {
			itemOnFloor->inventoryLetter = theItem->inventoryLetter;
			pickUpItemAt(player.xLoc, player.yLoc);
		}
		placeItem(theItem, player.xLoc, player.yLoc);
		return theItem;
	}
}

void recalculateEquipmentBonuses() {
	float enchant;
	item *theItem;
	
	if (rogue.weapon) {
		theItem = rogue.weapon;
		enchant = netEnchant(theItem);
		player.info.damage = theItem->damage;
		player.info.damage.lowerBound *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, enchant);
		player.info.damage.upperBound *= pow(WEAPON_ENCHANT_DAMAGE_FACTOR, enchant);
		if (player.info.damage.lowerBound < 1) {
			player.info.damage.lowerBound = 1;
		}
		if (player.info.damage.upperBound < 1) {
			player.info.damage.upperBound = 1;
		}
	}
	
	if (rogue.armor) {
		theItem = rogue.armor;
		enchant = netEnchant(theItem);
		player.info.defense = theItem->armor + enchant * 10;
		if (player.info.defense < 0) {
			player.info.defense = 0;
		}
	}
}

void equipItem(item *theItem, boolean force) {
	item *previouslyEquippedItem = NULL;
	
	if ((theItem->category & WEAPON) && (theItem->flags & ITEM_EQUIPPED) && rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER) {
		return;
	}
	
	if ((theItem->category & RING) && (theItem->flags & ITEM_EQUIPPED)) {
		return;
	}
	
	if ((theItem->category & WEAPON) && (!rogue.talisman || rogue.talisman->kind != TALISMAN_SINISTER)) {
		previouslyEquippedItem = rogue.weapon;
	} else if (theItem->category & ARMOR) {
		previouslyEquippedItem = rogue.armor;
	} else if (theItem->category & TALISMAN) {
		previouslyEquippedItem = rogue.talisman;
	} else if (theItem->category & SHIELD) {
		previouslyEquippedItem = rogue.shield;
	}
	if (previouslyEquippedItem) {
		if (!force && (previouslyEquippedItem->flags & ITEM_CURSED)) {
			return; // already using a cursed item
		} else if (theItem != previouslyEquippedItem) {
			unequipItem(previouslyEquippedItem, force);
		}
	}
	if (theItem->category & WEAPON) {
		if (rogue.weapon && rogue.offhandWeapon) {
			return;
		} else if (rogue.weapon && rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER) {
			rogue.offhandWeapon = theItem;
			rogue.offhandWeaponNext = false;
		} else {
			rogue.weapon = theItem;
			rogue.offhandWeaponNext = false;
		}
		recalculateEquipmentBonuses();
	} else if (theItem->category & ARMOR) {
		rogue.armor = theItem;
		recalculateEquipmentBonuses();
		updateRingBonuses(); // Needed because armor now modifies stealth.
		if (rogue.talisman && rogue.talisman->kind == TALISMAN_THIRD_EYE) {
			unequipItem(rogue.talisman, true);
		}
	} else if (theItem->category & TALISMAN) {
		rogue.talisman = theItem;
		rogue.talismanFraction = 100; // slightly penalize talisman swapping;
		switch(theItem->kind) {
			case TALISMAN_MADNESS:
				if (player.status[STATUS_HALLUCINATING]) {
					recalculateEquipmentBonuses();
				}
				break;
			case TALISMAN_SPIDER:
				if (player.status[STATUS_POISONED]) {
					updateRingBonuses();
				}
				break;
			case TALISMAN_FLAMESPIRIT:
				if (player.status[STATUS_BURNING]) {
					updateRingBonuses();
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_CLAIRVOYANCE) || (rogue.ringRight && rogue.ringRight->kind == RING_CLAIRVOYANCE)) {
						updateClairvoyance();
						displayLevel();
					}
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_TELEPATHY) || (rogue.ringRight && rogue.ringRight->kind == RING_TELEPATHY)) {
						updateTelepathy();
						displayLevel();
					}
				}
				break;
			case TALISMAN_SINISTER:
				if (rogue.shield) {
					unequipItem(rogue.shield, true);
				}
				break;
			case TALISMAN_THIRD_EYE:
				if (rogue.armor) {
					unequipItem(rogue.armor, force);
				}
				break;
		}
	} else if (theItem->category & SHIELD) {
		rogue.shield = theItem;
		if (rogue.talisman && rogue.talisman->kind == TALISMAN_SINISTER) {
			unequipItem(rogue.talisman, true);
		}
		recalculateEquipmentBonuses();
	} else if (theItem->category & RING) {
		if (rogue.ringLeft && rogue.ringRight && (!rogue.talisman || rogue.talisman->kind != TALISMAN_THIRD_EYE || rogue.ringThird)) {
			return;
		}
		if (rogue.ringLeft && rogue.ringRight) {
			rogue.ringThird = theItem;
		} else if (rogue.ringLeft) {
			rogue.ringRight = theItem;
		} else {
			rogue.ringLeft = theItem;
		}
		updateRingBonuses();
		if (theItem->kind == RING_CLAIRVOYANCE) {
			updateClairvoyance();
			displayLevel();
			ringTable[RING_CLAIRVOYANCE].identified = true;
			theItem->flags |= ITEM_IDENTIFIED;
		} else if (theItem->kind == RING_TELEPATHY) {
			updateTelepathy();
			displayLevel();
			ringTable[RING_TELEPATHY].identified = true;
		} else if (theItem->kind == RING_LIGHT) {
			ringTable[RING_LIGHT].identified = true;
		} else if (theItem->kind == RING_MIGHT) { // simplifies a lot of 'is this item too heavy?' code elsewhere
			ringTable[RING_MIGHT].identified = true;
			theItem->flags |= ITEM_IDENTIFIED;
		}
		if (theItem->enchant1 < 0 && rogue.talisman && rogue.talisman->kind == TALISMAN_BALANCE) {
			talismanTable[TALISMAN_BALANCE].identified = true;
		}
	}
	theItem->flags |= ITEM_EQUIPPED;
	return;
}

void unequipItem(item *theItem, boolean force) {
	creature *monst;
	short direction;
	
	if (theItem == NULL || !(theItem->flags & ITEM_EQUIPPED)) {
		return;
	}
	if ((theItem->flags & ITEM_CURSED) && !force) {
		return;
	}
	if ((theItem->category & TALISMAN) && theItem->kind == TALISMAN_ALCHEMY) {
		if (numberOfItemsInPackWithoutAlchemy() > MAX_PACK_ITEMS) {
			return;
		}
	}
	theItem->flags &= ~ITEM_EQUIPPED;
	if ((theItem->category & WEAPON) && rogue.weapon == theItem) {
		if (rogue.weapon->flags & ITEM_IMPALES) {
			direction = rogue.impaleDirection;
			rogue.impaleDirection = NO_DIRECTION;
			if (direction >= 0)
			{
				refreshDungeonCell(player.xLoc + nbDirs[direction][0], player.yLoc + nbDirs[direction][1]);
			}
		}
		rogue.weaponBloodstainColor = itemColor;
		player.info.damage.lowerBound = 1;
		player.info.damage.upperBound = 2;
		player.info.damage.clumpFactor = 1;
		rogue.weapon = rogue.offhandWeapon;
		rogue.offhandWeapon = NULL;
		rogue.offhandWeaponNext = false;
	} else if ((theItem->category & WEAPON) && rogue.offhandWeapon == theItem) {
		rogue.offhandWeapon = NULL;
		rogue.offhandWeaponNext = false;
	}
	if (theItem->category & ARMOR) {
		player.info.defense = 0;
		rogue.armor = NULL;
		updateRingBonuses(); // needed because armor modifies stealth
	}
	if (theItem->category & SHIELD) {
		rogue.shield = NULL;
	}
	if (theItem->category & RING) {
		if (rogue.ringLeft == theItem) {
			rogue.ringLeft = NULL;
		} else if (rogue.ringRight == theItem) {
			rogue.ringRight = NULL;
		} else if (rogue.ringThird == theItem) {
			rogue.ringThird = NULL;
		} 
		updateRingBonuses();
		if (theItem->kind == RING_CLAIRVOYANCE) {
			updateClairvoyance();
			updateClairvoyance(); // Yes, we have to call this twice.
			displayLevel();
		}
		if (theItem->kind == RING_TELEPATHY) {
			updateTelepathy();
			updateTelepathy(); // Well, I'm just going to assume the same.
			displayLevel();
		}
	}
	if (theItem->category & TALISMAN) {
		rogue.talisman = NULL;
		switch(theItem->kind) {
			case TALISMAN_MADNESS:
				if (player.status[STATUS_HALLUCINATING]) {
					recalculateEquipmentBonuses();
				}
				break;
			case TALISMAN_FLAMESPIRIT:
				if (player.status[STATUS_BURNING]) {
					updateRingBonuses();
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_CLAIRVOYANCE) || (rogue.ringRight && rogue.ringRight->kind == RING_CLAIRVOYANCE)) {
						updateClairvoyance();
						updateClairvoyance(); // Yes, we have to call this twice.
						displayLevel();
					}
					if ((rogue.ringLeft && rogue.ringLeft->kind == RING_TELEPATHY) || (rogue.ringRight && rogue.ringRight->kind == RING_TELEPATHY)) {
						updateTelepathy();
						updateTelepathy(); // Yes, we have to call this twice.
						displayLevel();
					}
				}
				break;
			case TALISMAN_SPIDER:
				for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
					if (monst->info.monsterID == MK_SPIDER && monst->creatureState == MONSTER_ALLY) {
						unAlly(monst);
					}
				}
				break;
			case TALISMAN_SACRIFICE:
				player.status[STATUS_BLESSED] = player.maxStatus[STATUS_BLESSED] = 0;
				break;
			case TALISMAN_SINISTER:
				if (rogue.offhandWeapon) {
					unequipItem(rogue.offhandWeapon, true);
				}
				break;
			case TALISMAN_THIRD_EYE:
				if (rogue.ringThird) {
					unequipItem(rogue.ringThird, true);
				}
				break;
			case TALISMAN_ASSASSIN:
				for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
					if (monst->bookkeepingFlags & MONST_ASSASSINATION_TARGET) {
						monst->bookkeepingFlags &= ~(MONST_ASSASSINATION_TARGET);
						monst->status[STATUS_MARKED] = monst->maxStatus[STATUS_MARKED] = 0;
						rogue.markedMonsters--;
						updateTelepathy();
						updateTelepathy();
						displayLevel();
					}
				}
				break;
		}
	}
	updateEncumbrance();
	return;
}

void updateRingBonuses() {
	short i, boost, j = 2;
	item *rings[3] = {rogue.ringLeft, rogue.ringRight, rogue.ringThird};
	
	rogue.clairvoyance = rogue.aggravating = rogue.stealthBonus
	= rogue.awarenessBonus = rogue.regenerationBonus = rogue.wisdomBonus
	= rogue.mightBonus = rogue.transference = rogue.telepathyBonus = 0;
	rogue.lightMultiplier = 1;
	
	if (rogue.talisman && rogue.talisman->kind == TALISMAN_THIRD_EYE) {
		j = 3;
		for (i=0; i< j; i++) {
		}
	}

	// Talisman of spiders boosts transference while poisoned
	if (rogue.talisman && rogue.talisman->kind == TALISMAN_SPIDER && player.status[STATUS_POISONED]) {
		rogue.transference += rogue.talisman->enchant1 * 2;
	}
	
	for (i=0; i< j; i++) {
		if (rings[i]) {
			
			boost = 0;
			
			// Talisman of balance boosts the cursed ring on the other hand
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_BALANCE) {
				if (rings[i]->enchant1 >= 0 && rings[(i+1)%2] && rings[(i+1)%2]->enchant1 < 0) {
					boost =  2 * -rings[(i+1)%2]->enchant1;
				}
			}
			
			// Talisman of flame dancing boosts all rings while on fire
			if (rogue.talisman && rogue.talisman->kind == TALISMAN_FLAMESPIRIT && player.status[STATUS_BURNING]) {
				boost += rogue.talisman->enchant1;
			}
			
			switch (rings[i]->kind) {
				case RING_CLAIRVOYANCE:
					rogue.clairvoyance += rings[i]->enchant1 + boost;
					break;
				case RING_STEALTH:
					rogue.stealthBonus += rings[i]->enchant1 + boost;
					break;
				case RING_REGENERATION:
					rogue.regenerationBonus += rings[i]->enchant1 + boost;
					break;
				case RING_LIGHT:
					rogue.lightMultiplier += rings[i]->enchant1 + boost;
					break;
				case RING_AWARENESS:
					rogue.awarenessBonus += 20 * (rings[i]->enchant1 + boost);
					break;
				case RING_WISDOM:
					rogue.wisdomBonus += rings[i]->enchant1 + boost;
					break;
				case RING_MIGHT:
					rogue.mightBonus += rings[i]->enchant1 + boost;
					break;
				case RING_TRANSFERENCE:
					rogue.transference += rings[i]->enchant1 + boost;
					break;
				case RING_TELEPATHY:
					rogue.telepathyBonus += rings[i]->enchant1 + boost;
					break;
			}
		}
	}
	
	if (rogue.lightMultiplier <= 0) {
		rogue.lightMultiplier--; // because it starts at positive 1 instead of 0
	}
	
	if (rogue.telepathyBonus > 0) {
		rogue.telepathyBonus += 2; // saves a lot of unnecessary checks
	} else if (rogue.telepathyBonus < 0) {
		rogue.telepathyBonus -= 2;
	}
	
	updateMinersLightRadius();
	updatePlayerRegenerationDelay();
	
	if (rogue.stealthBonus < 0) {
		rogue.stealthBonus *= 4;
		rogue.aggravating = true;
	}
	
	if (rogue.aggravating) {
		aggravateMonsters(0, 0, true, false, true);
	}
	
	if (rogue.armor) {
		rogue.stealthBonus += 2 - rogue.armor->kind;
	}
	else {
		rogue.stealthBonus += 3;
	}
}

void updatePlayerRegenerationDelay() {
	short maxHP;
	long turnsForFull; // In thousandths of a turn.
	maxHP = player.info.maxHP;
	turnsForFull = turnsForFullRegen(rogue.regenerationBonus);
	
	player.regenPerTurn = 0;
	while (maxHP > turnsForFull / 1000) {
		player.regenPerTurn++;
		maxHP -= turnsForFull / 1000;
	}
	
	player.info.turnsBetweenRegen = (turnsForFull / maxHP);
	// DEBUG printf("\nTurnsForFull: %i; regenPerTurn: %i; (thousandths of) turnsBetweenRegen: %i", turnsForFull, player.regenPerTurn, player.info.turnsBetweenRegen);
}

boolean removeItemFromChain(item *theItem, item *theChain) {
	item *previousItem;
	
	for (previousItem = theChain;
		 previousItem->nextItem;
		 previousItem = previousItem->nextItem) {
		if (previousItem->nextItem == theItem) {
			previousItem->nextItem = theItem->nextItem;
			return true;
		}
	}
	return false;
}

void deleteItem(item *theItem) {
	free(theItem);
}

void resetItemTableEntry(itemTable *theEntry) {
	theEntry->identified = false;
	theEntry->called = false;
	theEntry->callTitle[0] = '\0';
}

void shuffleFlavors() {
	short i, j, randIndex, randNumber;
	char buf[COLS];
	
	//	for (i=0; i<NUMBER_FOOD_KINDS; i++) {
	//		resetItemTableEntry(foodTable + i);
	//	}
	for (i=0; i<NUMBER_POTION_KINDS; i++) {
		resetItemTableEntry(potionTable + i);
	}
	for (i=0; i<NUMBER_ELIXIR_KINDS; i++) {
		resetItemTableEntry(elixirTable + i);
	}
	//	for (i=0; i<NUMBER_WEAPON_KINDS; i++) {
	//		resetItemTableEntry(weaponTable + i);
	//	}
	//	for (i=0; i<NUMBER_ARMOR_KINDS; i++) {
	//		resetItemTableEntry(armorTable + i);
	//	}
	for (i=0; i<NUMBER_STAFF_KINDS; i++) {
		resetItemTableEntry(staffTable+ i);
	}
	for (i=0; i<NUMBER_WAND_KINDS; i++) {
		resetItemTableEntry(wandTable + i);
	}
	for (i=0; i<NUMBER_SCROLL_KINDS; i++) {
		resetItemTableEntry(scrollTable + i);
	}
	for (i=0; i<NUMBER_TOME_KINDS; i++) {
		resetItemTableEntry(tomeTable + i);
	}
	for (i=0; i<NUMBER_RING_KINDS; i++) {
		resetItemTableEntry(ringTable + i);
	}
	for (i=0; i<NUMBER_TALISMAN_KINDS; i++) {
		resetItemTableEntry(talismanTable + i);
	}
	
	for (i=0; i<NUMBER_ITEM_COLORS; i++) {
		strcpy(itemColors[i], itemColorsRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_COLORS; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_COLORS - 1);
		if (itemColors[i] != itemColors[randIndex]) {
			strcpy(buf, itemColors[i]);
			strcpy(itemColors[i], itemColors[randIndex]);
			strcpy(itemColors[randIndex], buf);
		}
	}
	
	for (i=0; i<NUMBER_ITEM_CHEMISTRY; i++) {
		strcpy(itemChemistry[i], itemChemistryRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_CHEMISTRY; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_CHEMISTRY - 1);
		if (itemChemistry[i] != itemChemistry[randIndex]) {
			strcpy(buf, itemChemistry[i]);
			strcpy(itemChemistry[i], itemChemistry[randIndex]);
			strcpy(itemChemistry[randIndex], buf);
		}
	}
	
	for (i=0; i<NUMBER_ITEM_WOODS; i++) {
		strcpy(itemWoods[i], itemWoodsRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_WOODS; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_WOODS - 1);
		if (itemWoods[i] != itemWoods[randIndex]) {
			strcpy(buf, itemWoods[i]);
			strcpy(itemWoods[i], itemWoods[randIndex]);
			strcpy(itemWoods[randIndex], buf);
		}
	}
	
	for (i=0; i<NUMBER_ITEM_GEMS; i++) {
		strcpy(itemGems[i], itemGemsRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_GEMS; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_GEMS - 1);
		if (itemGems[i] != itemGems[randIndex]) {
			strcpy(buf, itemGems[i]);
			strcpy(itemGems[i], itemGems[randIndex]);
			strcpy(itemGems[randIndex], buf);
		}
	}
	
	for (i=0; i<NUMBER_ITEM_METALS; i++) {
		strcpy(itemMetals[i], itemMetalsRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_METALS; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_METALS - 1);
		if (itemMetals[i] != itemMetals[randIndex]) {
			strcpy(buf, itemMetals[i]);
			strcpy(itemMetals[i], itemMetals[randIndex]);
			strcpy(itemMetals[randIndex], buf);
		}
	}
	for (i=0; i<NUMBER_ITEM_MATERIALS; i++) {
		strcpy(itemMaterials[i], itemMaterialsRef[i]);
	}
	for (i=0; i<NUMBER_ITEM_MATERIALS; i++) {
		randIndex = rand_range(0, NUMBER_ITEM_MATERIALS - 1);
		if (itemMaterials[i] != itemMaterials[randIndex]) {
			strcpy(buf, itemMaterials[i]);
			strcpy(itemMaterials[i], itemMaterials[randIndex]);
			strcpy(itemMaterials[randIndex], buf);
		}
	}
	
	for (i=0; i<NUMBER_SCROLL_KINDS; i++) {
		itemTitles[i][0] = '\0';
		randNumber = rand_range(3, 4);
		for (j=0; j<randNumber; j++) {
			randIndex = rand_range(0, NUMBER_TITLE_PHONEMES - 1);
			strcpy(buf, itemTitles[i]);
			sprintf(itemTitles[i], "%s%s%s", buf, ((rand_percent(50) && j>0) ? " " : ""), titlePhonemes[randIndex]);
		}
	}
	for (i=0; i<NUMBER_TOME_KINDS; i++) {
		itemCovers[i][0] = '\0';
		randNumber = rand_range(3, 4);
		for (j=0; j<randNumber; j++) {
			randIndex = rand_range(0, NUMBER_COVER_PHRASES - 1);
			strcpy(buf, itemCovers[i]);
			sprintf(itemCovers[i], "%s%s%s", buf, ((rand_percent(50) && j>0) ? " " : ""), coverPhrases[randIndex]);
		}
	}	
}

unsigned long itemValue(item *theItem) {
	const short weaponRunicIntercepts[] = {
		700,	//W_SPEED,
		1000,	//W_QUIETUS,
		900,	//W_PARALYSIS,
		800,	//W_MULTIPLICITY,
		700,	//W_SLOWING,
		750,	//W_CONFUSION,
        850,    //W_FORCE,
		500,	//W_SLAYING,
		700,	//W_TRANSFERENCE,
		-1000,	//W_MERCY,
		-1000,	//W_PLENTY,
	};
	const short armorRunicIntercepts[] = {
		900,	//A_MULTIPLICITY,
		700,	//A_MUTUALITY,
		900,	//A_ABSORPTION,
		900,	//A_REPRISAL,
		500,	//A_IMMUNITY,
		900,	//A_REFLECTION,
		500,    //A_DAMPENING
		-1000,	//A_BURDEN,
		-1000,	//A_VULNERABILITY,
        -1000,  //A_IMMOLATION,
	};
	
	signed long value;
	
	switch (theItem->category) {
		case FOOD:
			return foodTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case WEAPON:
			value = (signed long) (weaponTable[theItem->kind].marketValue * theItem->quantity
								   * (1 + 0.15 * (theItem->enchant1 + (theItem->flags & ITEM_PROTECTED ? 1 : 0))));
			if (theItem->flags & ITEM_RUNIC) {
				value += weaponRunicIntercepts[theItem->enchant2];
				if (value < 0) {
					value = 0;
				}
			}
			return (unsigned long) value;
			break;
		case ARMOR:
			value = (signed long) (armorTable[theItem->kind].marketValue * theItem->quantity
								   * (1 + 0.15 * (theItem->enchant1 + ((theItem->flags & ITEM_PROTECTED) ? 1 : 0))));
			if (theItem->flags & ITEM_RUNIC) {
				value += armorRunicIntercepts[theItem->enchant2];
				if (value < 0) {
					value = 0;
				}
			}
			return (unsigned long) value;
			break;
		case SHIELD:
			value = (signed long) (shieldTable[theItem->kind].marketValue * theItem->quantity
								   * (1 + 0.15 * (theItem->enchant1 + ((theItem->flags & ITEM_PROTECTED) ? 1 : 0))));
			if (theItem->flags & ITEM_RUNIC) {
				value += armorRunicIntercepts[theItem->enchant2];
				if (value < 0) {
					value = 0;
				}
			}
			return (unsigned long) value;
			break;
		case SCROLL:
			return scrollTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case TOME:
			return tomeTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case POTION:
			return potionTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case ELIXIR:
			return elixirTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case STAFF:
			return staffTable[theItem->kind].marketValue * theItem->quantity
			* (float) (1 + 0.15 * (theItem->enchant1 - 1));
			break;
		case WAND:
			return wandTable[theItem->kind].marketValue * theItem->quantity;
			break;
		case RING:
			return ringTable[theItem->kind].marketValue * theItem->quantity
			* (float) (1 + 0.15 * (theItem->enchant1 - 1));
			break;
		case CHARM:
			return charmTable[theItem->kind].marketValue * theItem->quantity
			* (float) (1 + 0.15 * (theItem->enchant1 - 1));
			break;
		case TALISMAN:
			return talismanTable[theItem->kind].marketValue * theItem->quantity
			* (float) (1 + 0.15 * (theItem->enchant1 - 1));
			break;
		case AMULET:
			return 10000;
			break;
		case GEM:
			return 5000 * theItem->quantity;
			break;
		case KEY:
			return 0;
			break;
		default:
			return 0;
			break;
	}
}
