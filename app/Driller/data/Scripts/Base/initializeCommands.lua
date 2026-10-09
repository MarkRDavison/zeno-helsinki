cmd(GameCommand.new(DigShaftEvent.new(0), GameCommandContext.DiggingShaft, GameCommandSource.Setup))
cmd(GameCommand.new(DigShaftEvent.new(1), GameCommandContext.DiggingShaft, GameCommandSource.Setup))

cmd(GameCommand.new(DigTileEvent.new(0, 1), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 2), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 3), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 4), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 5), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 6), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(0, 7), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 1), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 2), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 3), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 4), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 5), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 6), GameCommandContext.DiggingTile, GameCommandSource.Setup))
cmd(GameCommand.new(DigTileEvent.new(1, 7), GameCommandContext.DiggingTile, GameCommandSource.Setup))

cmd(GameCommand.new(PlaceBuildingEvent.new("Building_Bunk", 0, 1), GameCommandContext.PlacingBuilding, GameCommandSource.Setup))

cmd(GameCommand.new(CreateWorkerEvent.new("Worker_Builder", vec2f.new(1.0, 0.0)), GameCommandContext.CreatingWorker, GameCommandSource.Setup))

cmd(GameCommand.new(CreateShuttleEvent.new("Shuttle_Basic"), GameCommandContext.CreatingShuttle, GameCommandSource.Setup))
