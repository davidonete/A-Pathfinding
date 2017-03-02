# A-Pathfinding
The simulator is composed by 30 agents that act by themselves. There are three different types of agents: soldiers, workers and guards.

The workers are prisoners for the guards and they spend time both carrying crates and resting, while the guards patrol the fortress. The soldiers start from their base and try to rescue the workers.  All the agents are controlled by a state machine different for each one of them. Each state may have sub states (to define small behaviours inside of a task).

The agents move by both random movement and pathfinding, using the A* algorithm.
