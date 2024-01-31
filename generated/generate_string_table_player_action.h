// examples/turn_based/game_types.h:111:0

link_internal counted_string
ToString(player_action Type)
{
  counted_string Result = {};
  switch (Type)
  {
    case PlayerAction_None: { Result = CSz("PlayerAction_None"); } break;
    case PlayerAction_Move: { Result = CSz("PlayerAction_Move"); } break;
    case PlayerAction_Shovel: { Result = CSz("PlayerAction_Shovel"); } break;
    case PlayerAction_Grab: { Result = CSz("PlayerAction_Grab"); } break;
    case PlayerAction_ChargeFireball: { Result = CSz("PlayerAction_ChargeFireball"); } break;
    case PlayerAction_IceBlock: { Result = CSz("PlayerAction_IceBlock"); } break;
    case PlayerAction_Throw: { Result = CSz("PlayerAction_Throw"); } break;
    case PlayerAction_Count: { Result = CSz("PlayerAction_Count"); } break;

    
  }
  return Result;
}

