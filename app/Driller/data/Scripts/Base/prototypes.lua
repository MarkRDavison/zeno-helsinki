basicCalculateOffset = 
function (jobInstance, jobPrototype)
    if (jobInstance.tile.x > 0)
    then
        return vec2f.new(-0.5,0.0)
    end
    return vec2f.new(0.5,0.0)
end

prototypes = {
    shuttles = {
        {
            name = "Shuttle_Basic",
            capacity = 25,
            loadingTime = 5.0,
            size = {
                x = 3,
                y = 2
            },
            texture = {
                x = 0,
                y = 5
            },
            idleTime = 25.0,
            speed = 25.0,
            allowedCargo = {
                "Resource_Ore"
            }
        }
    },
	workers = {
        {
            name = "Worker_Builder",
            jobs = {
                "Job_Dig",
                "Job_Build_Building"
            }
        },
        {
            name = "Worker_Miner",
            jobs = {
                "Job_Mine"
            }
        },
        {
            name = "Worker_Refiner",
            jobs = {
                "Job_Refine"
            }
        }
    },
    jobs = {
        {
            name = "Job_DigShaft",
            repeats = false,
            work = 0.0,
            onComplete =
                function (jobInstance)
                    cmd(GameCommand.new(DigShaftEvent.new(jobInstance.tile.y), GameCommandContext.DiggingShaft, GameCommandSource.System))
                end
        },
        {
            name = "Job_Dig",
            repeats = false,
            work = 2.0,
            onComplete =
                function (jobInstance)
                    cmd(GameCommand.new(DigTileEvent.new(jobInstance.tile.y, jobInstance.tile.x), GameCommandContext.DiggingTile, GameCommandSource.System))
                end,
            calculateOffset = basicCalculateOffset
        },
        {
            name = "Job_Mine",
            repeats = true,
            work = 4.0,
            onComplete = 
                function (jobInstance)
                    cmd(GameCommand.new(AddResourceEvent.new("Resource_Ore", jobInstance.tile.y + 1), GameCommandContext.AddResource, GameCommandSource.System))
                end
        },
        {
            name = "Job_Build_Building",
            repeats = false,
            work = 5.0 ,
            onComplete = 
                function (jobInstance)
                    cmd(GameCommand.new(PlaceBuildingEvent.new(jobInstance.additionalPrototypeId, jobInstance.tile.y, jobInstance.tile.x), GameCommandContext.PlacingBuilding, GameCommandSource.System))
                end
        },
        {
            name = "Job_Refine",
            repeats = true,
            work = 32.0,
            onComplete = 
                function (jobInstance)
                    cmd(GameCommand.new(AddUpgradeEvent.new("Upgrade_Refine", 0.001), GameCommandContext.AddingUpgrade, GameCommandSource.System))
                end
        }
    },
    buildings = {
        {
            name = "Building_Bunk",
            size = {
                x = 2,
                y = 1
            },
            texture = {
                x = 3,
                y = 0
            },
            workers = {
            }
        },
        {
            name = "Building_Builders_Hut",
            size = {
                x = 2,
                y = 1
            },
            texture = {
                x = 5,
                y = 0
            },
            workers = {
                {
                    name = "Worker_Builder",
                    amount = 2
                }
            }
        },
        {
            name = "Building_Mine",
            size = {
                x = 3,
                y = 1
            },
            texture = {
                x = 7,
                y = 0
            },
            workers = {
                {
                    name = "Worker_Miner",
                    amount = 1
                }
            },
            jobs = {
                {
                    name = "Job_Mine",
                    offset = {
                        x = 1.0,
                        y = 0.0
                    }
                }
            }
        },
        {
            name = "Building_Refining",
            size = {
                x = 4,
                y = 1
            },
            texture = {
                x = 10,
                y = 0
            },
            workers = {
                {
                    name = "Worker_Refiner",
                    amount = 2
                }
            },
            jobs = {
                {
                    name = "Job_Refine",
                    offset = {
                        x = 0.5,
                        y = 0.0
                    }
                },
                {
                    name = "Job_Refine",
                    offset = {
                        x = 2.5,
                        y = 0.0
                    }
                }
            }
        }
    }
}
