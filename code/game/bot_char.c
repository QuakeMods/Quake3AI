// bot_char.c
//

#include "g_local.h"
#include "bot_local.h"

/*
============================
BotFreeCharacterStrings
============================
*/
void BotFreeCharacterStrings(bot_character_t* ch)
{
	int i;

	for (i = 0; i < MAX_CHARACTERISTICS; i++)
	{
		if (ch->c[i].type == CT_STRING)
		{
			//FreeMemory(ch->c[i].value.string); // jmarshall fix me
		}
	} 
} 

/*
============================
BotLoadCharacterFromFile
============================
*/
bot_character_t* BotLoadCharacterFromFile(char* charfile, int skill)
{
	int indent, index, foundcharacter;
	bot_character_t* ch;
	int source;
	pc_token_t token;

	foundcharacter = qfalse;
	
	//a bot character is parsed in two phases
	trap_PC_SetBaseFolder(BOTFILESBASEFOLDER);
	source = trap_PC_LoadSource(charfile);
	if (!source)
	{
		G_Printf("counldn't load %s\n", charfile);
		return NULL;
	} 

	ch = (bot_character_t*)malloc(sizeof(bot_character_t) + MAX_CHARACTERISTICS * sizeof(bot_characteristic_t));
	memset(ch, 0, sizeof(bot_character_t) + MAX_CHARACTERISTICS * sizeof(bot_characteristic_t));
	strcpy(ch->filename, charfile);
	while (trap_PC_ReadToken(source, &token))
	{
		if (!strcmp(token.string, "skill"))
		{
			if (!trap_PC_ExpectTokenType(source, TT_NUMBER, 0, &token))
			{
				trap_PC_FreeSource(source);
				BotFreeCharacterStrings(ch);
				//FreeMemory(ch);
				return NULL;
			}

			if (!trap_PC_ExpectTokenString(source, "{"))
			{
				trap_PC_FreeSource(source);
				BotFreeCharacterStrings(ch);
				//FreeMemory(ch);
				return NULL;
			}

			//if it's the correct skill
			if (skill < 0 || token.intvalue == skill)
			{
				foundcharacter = qtrue;
				ch->skill = token.intvalue;
				while (trap_PC_ReadToken(source, &token))
				{
					if (!strcmp(token.string, "}")) 
						break;
					
					if (token.type != TT_NUMBER || !(token.subtype & TT_INTEGER))
					{
						G_Error("expected integer index, found %s\n", token.string);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
					//	FreeMemory(ch);
						return NULL;
					}

					index = token.intvalue;
					if (index < 0 || index > MAX_CHARACTERISTICS)
					{
						G_Error("characteristic index out of range [0, %d]\n", MAX_CHARACTERISTICS);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					}

					if (ch->c[index].type)
					{
						G_Error("characteristic %d already initialized\n", index);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					}

					if (!trap_PC_ReadToken(source, &token))
					{
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
					//	FreeMemory(ch);
						return NULL;
					}

					if (token.type == TT_NUMBER)
					{
						if (token.subtype & TT_FLOAT)
						{
							ch->c[index].value._float = token.floatvalue;
							ch->c[index].type = CT_FLOAT;
						} 
						else
						{
							ch->c[index].value.integer = token.intvalue;
							ch->c[index].type = CT_INTEGER;
						} 
					}
					else if (token.type == TT_STRING)
					{
						StripDoubleQuotes(token.string);
						ch->c[index].value.string = G_Alloc(strlen(token.string) + 1);
						strcpy(ch->c[index].value.string, token.string);
						ch->c[index].type = CT_STRING;
					}
					else
					{
						G_Error("expected integer, float or string, found %s\n", token.string);
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					}
				} 
				break;
			} 
			else
			{
				indent = 1;
				while (indent)
				{
					if (!trap_PC_ReadToken(source, &token))
					{
						trap_PC_FreeSource(source);
						BotFreeCharacterStrings(ch);
						//FreeMemory(ch);
						return NULL;
					} 
					if (!strcmp(token.string, "{")) 
						indent++;
					else if (!strcmp(token.string, "}")) 
						indent--;
				} 
			} 
		} 
		else
		{
			G_Error("unknown definition %s\n", token.string);
			trap_PC_FreeSource(source);
			BotFreeCharacterStrings(ch);
			//FreeMemory(ch);
			return NULL;
		} 
	} 
	trap_PC_FreeSource(source);
	
	if (!foundcharacter)
	{
		BotFreeCharacterStrings(ch);
		//FreeMemory(ch);
		return NULL;
	}
	return ch;
} 