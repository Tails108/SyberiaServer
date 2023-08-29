modded class ActionSkinning
{
	// Spawns the loot according to the Skinning class in the dead agent's config
	override void OnFinishProgressServer( ActionData action_data )
	{
		float knifeDmgMultiplier = 1;
		Object targetObject = action_data.m_Target.GetObject();		
		if (targetObject && targetObject.IsInherited(AnimalBase))
		{
			AnimalBase body_EB = AnimalBase.Cast(targetObject);
			body_EB.SetAsSkinned();
			body_EB.DoSkinning(action_data.m_Player, action_data.m_MainItem);
			knifeDmgMultiplier = targetObject.ConfigGetFloat("knifeDamageModifier");
		}
		else if (targetObject && targetObject.IsInherited(SurvivorBase))
		{
			SurvivorBase body_SB = SurvivorBase.Cast(targetObject);
			body_SB.SetAsSkinned();
			body_SB.DoSkinning(action_data.m_Player, action_data.m_MainItem);
			knifeDmgMultiplier = targetObject.ConfigGetFloat("knifeDamageModifier");
			// This section drops all clothes (and attachments) from the dead player before deleting their body
			if (body_SB.IsRestrained() && body_SB.GetHumanInventory().GetEntityInHands())
			{
				MiscGameplayFunctions.TransformRestrainItem(body_SB.GetHumanInventory().GetEntityInHands(), null, action_data.m_Player, body_SB);
				/*
				EntityAI item_in_hands = action_data.m_MainItem;
				body_SB.SetRestrained(false);
				string new_item_name = MiscGameplayFunctions.ObtainRestrainItemTargetClassname(item_in_hands);			
				MiscGameplayFunctions.TurnItemInHandsIntoItemEx(body_SB, new UnrestrainSelfPlayer(item_in_hands, new_item_name));
				*/
			}
			//Remove splint if target is wearing one
			if (body_SB.IsWearingSplint())
			{
				EntityAI entity = action_data.m_Player.SpawnEntityOnGroundOnCursorDir("Splint", 0.5);
				EntityAI attachment;
				ItemBase new_item = ItemBase.Cast(entity);
				Class.CastTo(attachment, body_SB.GetItemOnSlot("Splint_Right"));
				if ( attachment && attachment.GetType() == "Splint_Applied" )
				{
					if (new_item)
					{
						MiscGameplayFunctions.TransferItemProperties(attachment,new_item);
						
						if (GetGame().IsServer())
						{
							//Lower health level of splint after use
							if (new_item.GetHealthLevel() < 4)
							{
								int newDmgLevel = new_item.GetHealthLevel() + 1;
								
								float max = new_item.GetMaxHealth("","");
								
								switch ( newDmgLevel )
								{
									case GameConstants.STATE_BADLY_DAMAGED:
										new_item.SetHealth( "", "", max * GameConstants.DAMAGE_BADLY_DAMAGED_VALUE );
										break;
				
									case GameConstants.STATE_DAMAGED:
										new_item.SetHealth( "", "", max * GameConstants.DAMAGE_DAMAGED_VALUE );
										break;
				
									case GameConstants.STATE_WORN:
										new_item.SetHealth( "", "", max * GameConstants.DAMAGE_WORN_VALUE );
										break;
									
									case GameConstants.STATE_RUINED:
										new_item.SetHealth( "", "", max * GameConstants.DAMAGE_RUINED_VALUE );
										break;
									
									default:
										break;
								}
							}
						}
					}
					attachment.Delete();
				}
			}
			
			/*
			DropEquipAndDestroyRootLambda lambda(body_SB, "", action_data.m_Player);
			action_data.m_Player.ServerReplaceItemWithNew(lambda);
			*/
			int deadBodyLifetime;
			if ( GetCEApi() )
			{
				deadBodyLifetime = GetCEApi().GetCEGlobalInt( "CleanupLifetimeDeadPlayer" );
				if ( deadBodyLifetime <= 0 )
				{
					deadBodyLifetime = 3600;
				}
			}
			array<EntityAI> children = new array<EntityAI>;
			body_SB.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, children);
			int count = children.Count();
			for (int i = 0; i < count; ++i)
			{
				EntityAI child = children.Get(i);
				if ( child )
				{
					body_SB.GetInventory().DropEntity(InventoryMode.SERVER, body_SB, child);
					child.SetLifetime( deadBodyLifetime );
				}
			}
		}
		else if (targetObject && targetObject.IsInherited(Edible_Base))
		{
			Edible_Base edibleBase = Edible_Base.Cast(targetObject);
			edibleBase.DoSkinning(action_data.m_Player, action_data.m_MainItem);
			knifeDmgMultiplier = targetObject.ConfigGetFloat("knifeDamageModifier");
		}
		
		if (knifeDmgMultiplier <= 0)
		{
			knifeDmgMultiplier = 1;
		}
		
		if (action_data.m_MainItem)
		{
			float skill = 1 - action_data.m_Player.GetPerkFloatValue(SyberiaPerkType.SYBPERK_HUNTING_TOOLS, 0, 0);
			float dmgValue = GetSyberiaConfig().m_animalsButchingKnifeDamage;
			float decreaseHealthValue = skill * dmgValue * knifeDmgMultiplier;
			MiscGameplayFunctions.DealAbsoluteDmg(action_data.m_MainItem, decreaseHealthValue);
			targetObject.Delete();
		}
		
		ItemBase gloves = action_data.m_Player.GetItemOnSlot("Gloves");
        if (gloves)
        {
            gloves.SetCleanness(0);
        }
        else
        {
            action_data.m_Player.SetBloodyHands(true);
        }
	}
};