# AIthello

## Overview

This project aims at creating a good AI for the Othello game. 

It explores various techniques for creating a good AI 
such as minmax tree research (with optimization) with simple hand made heuristics
or neuronal network powered heuristics
but also prefer tree search for variating height research.

This project is still on going and lacks many features such as MonteCarlo tree search.

## What is Othello ?

### Principle

This a board game for 2 players where the objective is to have as many pieces of your color as possible. The twist is that your pieces can be flipped (therefore swithcing colors) if your opponent traps it between two of their pieces.

You can find all the rules on [Wikipedia](https://en.wikipedia.org/wiki/Reversi). If you are french, I encourage you to check the [FFO website](https://www.ffothello.org/othello/regles-du-jeu/).

### What interest ?

It is a difficult game as the position can really quickly change. There is no simple way to evaluate a position and there are too many possibilities to explore them all.

This is why this game has interested many programmers which have developped very good AI (for example [Egaroucid](https://github.com/Nyanyan/Egaroucid)).

## My approch

The AI creation has gone through many steps :
 1. Creating a minmax tree search algorithm and optimizing it with alphabeta cut.
 2. Creating a prefer tree search algorithm (exploring first the branch with the best winning probability).
 3. Testing simple hand made heuristics (score based ones and mobility based ones).
 4. Creating heuristics through machine learning : 
    - Articially augmenting the research by fitting a minimax evaluation.
    - Learning by copying master moves.

This project is mainly me trying to understand in depth AI training and tree search algorithm by testing diverse approches. 

### Note on Prefer tree search

It is this idea that gave the motivation to start this project : I really wanted to test it !!

The idea fo this tree search algorithm is to follow at each step the most probable path. 

For this, you need a probability function (in my case, I'm using softmax based on my heuristics) and then calculate the probability of a branch by multiplying the probability of each ropes alongs the branch. To get the final evaluation, you just use a minmax tree search on the created tree.

This allows to explore more in depth the most interesting part of the tree while still exploring early branches as the products favores the nodes nearest to the source. 

### Note on AI training



## Testing the project

To build the project, I recommand downloading the all project (keeping the structure)
and using the CMakeLists.txt file to compile it.

If you want, you can change the variable `IS_GRAPH` to get a graphical interface 
and `IS_GEN` to generate train data.
However, I would recommand using the non graphical interface as it is the most polished.

When launching the project, you will be faced with a terminal like interface.

This functions just like a terminal. The main commands are `play` and `train [nb_pass] [size_batch]` and `test`.

If you are lost, please use `help` or `help [cmd]` to get details.

## Acknowledgements

I have found a lot of usefull information on the [FFO website](https://www.ffothello.org/) concerning basics [strategies](https://www.ffothello.org/othello/principes-strategiques/) and [common algorithms](https://www.ffothello.org/othello/principes-strategiques/).


The random generator used in this project is the xoroshiro128+ PRNG. It has encapsulated in a class `Random` to make switching PRNG as easy as possible (for example for a Mersenne Twister).


The code uses two auxiliary librairies :
 - `Terminal.hpp` : a librairy I created to make customizable terminals.
 - `NL` : a librairy I created for creating neuronal networks and training them. It lacks many optimizations but is functional.
