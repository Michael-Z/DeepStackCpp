#include "TreeLookahead.h"


TreeLookahead::TreeLookahead(Node& root, long long skip_iters, long long iters)
{
	_cfr_skip_iters = skip_iters;
	_cfr_iters = iters;
	_root = root;
}

TreeLookahead::~TreeLookahead()
{

}

void TreeLookahead::resolve_first_node(const Range& player_range, const Range& opponent_range)
{
	assert(player_range.size() > 0);
	assert(opponent_range.size() > 0);
	assert(_cfr_iters >= _cfr_skip_iters);
	_root.ranges.row(P1) = player_range;
	_root.ranges.row(P2) = opponent_range;
	_compute();
}

void TreeLookahead::resolve(const Range& player_range, const Range& opponent_cfvs)
{
	_reconstruction_gadget = new cfrd_gadget_f(_root.board, player_range, opponent_cfvs);
	_reconstruction_opponent_cfvs = opponent_cfvs;
	_reconstruction = true;
	_compute();
}

Tf1 TreeLookahead::get_chance_action_cfv(int action_index, Tf1& board)
{
	return Tf1();
}

LookaheadResult TreeLookahead::get_results()
{
	//LookaheadResult out;

	//const int actions_count = (int)average_strategies_data[1].dimension(0);

	////--1.0 average strategy
	////--[actions x range]
	////--lookahead already computes the average strategy we just convert the dimensions
	//const std::array<DenseIndex, 2> action_cards_dims = { actions_count, card_count };
	//out.strategy = average_strategies_data[1].reshape(action_cards_dims);

	////--2.0 achieved opponent's CFVs at the starting node 
	//const std::array<DenseIndex, 2> players_cards_dims = { players_count, card_count };
	//Tf2 resized_average_cfvs = average_cfvs_data[0].reshape(players_cards_dims); //ToDo: copy

	//out.achieved_cfvs = RemoveF1D(resized_average_cfvs, P1);

	////--3.0 CFVs for the acting player only when resolving first node
	//if (_reconstruction_opponent_cfvs.size() > 0)
	//{
	//	out.root_cfvs = RemoveF1D(resized_average_cfvs, P2);

	//	//--swap cfvs indexing
	//	out.root_cfvs_both_players = resized_average_cfvs; // !Copy Do we need this? Looks like we are overwriting below
	//	RemoveF1D(out.root_cfvs_both_players, P2) = out.achieved_cfvs;
	//	RemoveF1D(out.root_cfvs_both_players, P1) = out.root_cfvs;
	//}
	//else
	//{
	//	assert(false);
	//}

	////--4.0 children CFVs
	////--[actions x range]
	//out.children_cfvs = Remove4D(average_cfvs_data[1], P1).reshape(action_cards_dims);

	////--IMPORTANT divide average CFVs by average strategy in here
	//Tf2 scaler = average_strategies_data[1].reshape(action_cards_dims);

	//const Eigen::array<DenseIndex, 2> s_dims = { 1, card_count };

	//Tf2 range_mul = Remove4D(ranges_data[0], P1).reshape(s_dims);
	//range_mul = Util::ExpandAs(range_mul, scaler);

	//scaler *= range_mul;
	//Tf2 scalerSum = Util::NotReduceSum(scaler, 1);

	//scaler = Util::ExpandAs(scalerSum, range_mul);
	//scaler *= scaler.constant((float)(_cfr_iters - _cfr_skip_iters));

	//out.children_cfvs /= scaler;

	//assert(out.strategy.size() > 0);
	//assert(out.achieved_cfvs.size() > 0);
	//assert(out.children_cfvs.size() > 0);

	//return out;

	return LookaheadResult();
}

void TreeLookahead::_compute()
{
	//--1.0 main loop
	for (size_t iter = 0; iter < _cfr_iters; iter++)
	{
		if (_reconstruction)
		{
			_set_opponent_starting_range();
		}

		cfrs_iter_dfs(_root, iter);

		if (iter >= _cfr_skip_iters)
		{
			_compute_update_average_strategies();
			_compute_cumulate_average_cfvs();
		}
	}

	//--2.0 at the end normalize average strategy
	_compute_normalize_average_strategies();
	//--2.1 normalize root's CFVs
	_compute_normalize_average_cfvs();
}

void TreeLookahead::_set_opponent_starting_range()
{
	_root.ranges.row(P2) = _reconstruction_gadget->compute_opponent_range(_reconstruction_opponent_cfvs);
}

void TreeLookahead::_compute_current_strategies()
{

}

void TreeLookahead::_compute_ranges()
{

}

void TreeLookahead::_compute_normalize_average_cfvs()
{

}

void TreeLookahead::_compute_terminal_equities_next_street_box()
{

}

void TreeLookahead::_compute_update_average_strategies()
{

}

void TreeLookahead::_compute_terminal_equities_terminal_equity()
{

}

void TreeLookahead::_processSecondStreetTermEq(int d, Tf2& csvfs_res)
{

}

void TreeLookahead::_processFirstStreetTermEq(int d, Tf2& csvfs_res)
{

}

void TreeLookahead::_compute_terminal_equities()
{

}

void TreeLookahead::_compute_cfvs()
{

}

void TreeLookahead::_compute_cumulate_average_cfvs()
{

}

void TreeLookahead::_compute_normalize_average_strategies()
{

}

void TreeLookahead::_compute_regrets()
{

}


//------------------------------------------

void TreeLookahead::cfrs_iter_dfs(Node& node, size_t iter)
{
	assert(node.current_player == P1 || node.current_player == P2 || node.current_player == chance);
	//--compute values using terminal_equity in terminal nodes
	if (node.terminal)
	{
		_fillCFvaluesForTerminalNode(node);
	}
	else
	{
		_fillCFvaluesForNonTerminalNode(node, iter);
	}
}

void TreeLookahead::_fillCFvaluesForTerminalNode(Node &node)
{
	assert(node.terminal && (node.type == terminal_fold || node.type == terminal_call));
	int opponnent = 1 - node.current_player;

	terminal_equity* termEquity = _get_terminal_equity(node);

	// CF values  2p X each private hand.
	node.cf_values = ArrayXX::Zero(players_count, card_count);

	if (node.type == terminal_fold)
	{
		termEquity->tree_node_fold_value(node.ranges, node.cf_values, opponnent);
	}
	else
	{
		termEquity->tree_node_call_value(node.ranges, node.cf_values);
	}

	//--multiply by the pot
	node.cf_values *= node.pot;

	//Looks like this not needed? ToDo: check during debugging!
	//node.cf_values.conservativeResize(node.ranges_absolute.rows(), node.ranges_absolute.cols());
}


void TreeLookahead::_fillCFvaluesForNonTerminalNode(Node &node, size_t iter)
{
	const int actions_count = (int)node.children.size();
	ArrayXX current_strategy;


	//--current cfv[[actions, players, ranges]]
	//[actions_count x card_count][players_count]
	CFVS cf_values_allactions[players_count];
	cf_values_allactions[P1] = CFVS(actions_count, card_count);
	cf_values_allactions[P2] = CFVS(actions_count, card_count);

	//map <player/[actions, ranges]>
	map <int, ArrayXX> children_ranges_absolute;

	static const int PLAYERS_DIM = 1;

	//cf_values_allactions.setZero();

	if (node.current_player == chance)
	{
		_fillChanceRangesAndStrategy(node, children_ranges_absolute, current_strategy);
	}
	else
	{
		_fillPlayersRangesAndStrategy(node, children_ranges_absolute, current_strategy);
	}

	for (size_t i = 0; i < node.children.size(); i++)
	{
		Node* child_node = node.children[i];
		//--set new absolute ranges(after the action) for the child
		child_node->ranges = ArrayXX(node.ranges);

		child_node->ranges.row(P1) = children_ranges_absolute[P1].row(i);
		child_node->ranges.row(P2) = children_ranges_absolute[P2].row(i);
		cfrs_iter_dfs(*child_node, iter);

		// Now coping cf_values from children to calculate the regret
		cf_values_allactions[P1].row(i) = Range(child_node->cf_values.row(P1)); //ToDo: Can be single copy operation(two rows copy)? ToDo:remove convention to Range
		cf_values_allactions[P2].row(i) = Range(child_node->cf_values.row(P2)); // ToDo:remove convention to Range
	}

	node.cf_values = Range(players_count, card_count);// ToDo:remove convention to Range
	node.cf_values.fill(0);

	Tensor<float, 2> tempVar;

	if (node.current_player == chance)
	{
		// For the chance node just sum regrets from all cards
		node.cf_values.row(P1) = cf_values_allactions[P1].colwise().sum();
		node.cf_values.row(P2) = cf_values_allactions[P2].colwise().sum();
		//node.cf_values.row(0) = Util::TensorToArray2d(cf_values_allactions, 0, PLAYERS_DIM, tempVar).colwise().sum();
		//node.cf_values.row(1) = Util::TensorToArray2d(cf_values_allactions, 1, PLAYERS_DIM, tempVar).colwise().sum();
	}
	else
	{
		ArrayXX current_regrets = ComputeRegrets(node, current_strategy, cf_values_allactions);
		update_regrets(node, current_regrets);
		//--accumulating average strategy
		update_average_strategy(node, current_strategy, iter);
	}
}

terminal_equity* TreeLookahead::_get_terminal_equity(Node& node)
{
	auto it = _cached_terminal_equities.find(&node.board);

	terminal_equity* cached = nullptr;
	if (it == _cached_terminal_equities.end())
	{
		cached = new terminal_equity();
		cached->set_board(node.board);
		_cached_terminal_equities[&node.board] = cached;
	}
	else
	{
		cached = it->second;
	}

	return cached;
}

void TreeLookahead::update_regrets(Node& node, const ArrayXX& current_regrets)
{
	//--node.regrets:add(current_regrets)
	//	--local negative_regrets = node.regrets[node.regrets:lt(0)]
	//	--node.regrets[node.regrets:lt(0)] = negative_regrets
	node.regrets.array() += current_regrets;

	node.regrets = (node.regrets.array() >= regret_epsilon).select(
		node.regrets,
		MatrixXd::Constant(node.regrets.rows(), node.regrets.cols(), regret_epsilon));
}

void TreeLookahead::update_average_strategy(Node& node, ArrayXX& current_strategy, size_t iter)
{
	if (iter >= _cfr_skip_iters)
	{
		if (node.strategy.size() == 0)
		{
			const int actions_count = (int)node.children.size();
			node.strategy = ArrayXX::Zero(actions_count, card_count);
		}

		if (node.iter_weight_sum.size() == 0)
		{
			node.iter_weight_sum = ArrayX::Zero(card_count);
		}

		ArrayX iter_weight_contribution = node.ranges.row(node.current_player);
		Util::ClipLow(iter_weight_contribution, regret_epsilon);
		node.iter_weight_sum += iter_weight_contribution;

		ArrayXX iter_weight = iter_weight_contribution / node.iter_weight_sum;
		iter_weight.resize(1, card_count);
		ArrayXX expanded_weight = Util::ExpandAs(iter_weight, node.strategy);
		ArrayXX old_strategy_scale = (expanded_weight * (-1)) + 1; //--same as 1 - expanded weight
		node.strategy *= old_strategy_scale;
		node.strategy += (current_strategy  * expanded_weight);
	}
}

void TreeLookahead::_fillChanceRangesAndStrategy(Node &node, map<int, ArrayXX> &children_ranges_absolute, ArrayXX& current_strategy)
{
	int actions_count = (int)node.children.size();
	current_strategy = node.strategy;

	ArrayXX ranges_mul_matrix = node.ranges.row(0).replicate(actions_count, 1);
	children_ranges_absolute[0] = current_strategy * ranges_mul_matrix;

	ranges_mul_matrix = node.ranges.row(1).replicate(actions_count, 1);
	children_ranges_absolute[1] = current_strategy * ranges_mul_matrix;
}

void TreeLookahead::_fillPlayersRangesAndStrategy(Node &node, map<int, ArrayXX> &children_ranges_absolute, ArrayXX& current_strategy)
{
	int currentPlayer = node.current_player; // Because we have zero based indexes, unlike the original source.
	int opponentIndex = 1 - currentPlayer;

	const int actions_count = (int)node.children.size();

	//--we have to compute current strategy at the beginning of each iteration

	//--initialize regrets in the first iteration
	if (node.regrets.size() == 0)
	{
		node.regrets = MatrixX(actions_count, card_count);
		node.regrets.fill(regret_epsilon);
	}

	assert((node.regrets >= regret_epsilon).all() && "All regrets must be positive or uncomment commented code below.");

	//--compute the current strategy
	ArrayXX regrets_sum = node.regrets.colwise().sum().row(action_dimension);
	current_strategy = ArrayXX(node.regrets);
	current_strategy /= Util::ExpandAs(regrets_sum, current_strategy); // We are dividing regrets for each actions by the sum of regrets for all actions and doing this element wise for every card

	ArrayXX ranges_mul_matrix = node.ranges.row(currentPlayer).replicate(actions_count, 1);
	children_ranges_absolute[currentPlayer] = current_strategy.array() * ranges_mul_matrix.array(); // Just multiplying ranges(cards probabilities) by the probability that action will be taken(from the strategy) inside the matrix  
	children_ranges_absolute[opponentIndex] = node.ranges.row(opponentIndex).replicate(actions_count, 1); //For opponent we are just cloning ranges
}

ArrayXX TreeLookahead::ComputeRegrets(Node &node, ArrayXX &current_strategy, ArrayXX * cf_values_allactions)
{
	Tensor<float, 2> tempVar;
	static const int PLAYERS_DIM = 1;
	const int actions_count = (int)node.children.size();
	const int currentPlayer = node.current_player; // Because we have zero based indexes, unlike the original source.
	const int opponent = 1 - node.current_player;

	assert(current_strategy.rows() == actions_count && current_strategy.cols() == card_count);
	//current_strategy.conservativeResize(actions_count, card_count); // Do we need this?

	//Map<ArrayXX> opCfAr = Util::TensorToArray2d(cf_values_allactions, opponentIndexNorm, PLAYERS_DIM, tempVar);

	ArrayXX opCfAr = cf_values_allactions[opponent]; // [actions X cards] - cf values for the opponent
	node.cf_values.row(opponent) = opCfAr.colwise().sum(); // for opponent assume that strategy is uniform

														   //ArrayXX strategy_mul_matrix = current_strategy; //ToDo: remove or add copy
														   //Map<ArrayXX> plCfAr = Util::TensorToArray2d(cf_values_allactions, currentPlayerNorm, PLAYERS_DIM, tempVar);

	ArrayXX currentPlayerCfValues = cf_values_allactions[currentPlayer];
	assert(currentPlayerCfValues.rows() == actions_count && currentPlayerCfValues.cols() == card_count);

	ArrayXX weigtedCfValues = current_strategy * currentPlayerCfValues; // weight the regrets by the used strategy
	node.cf_values.row(currentPlayer) = weigtedCfValues.colwise().sum(); // summing CF values for different actions

																		 //--computing regrets
																		 //Map<ArrayXX> current_regrets = Util::TensorToArray2d(cf_values_allactions, currentPlayerNorm, PLAYERS_DIM, tempVar);

																		 //current_regrets.resize(actions_count, card_count); Do we need this resize?


	ArrayXX cfValuesOdCurrentPlayer = node.cf_values.row(currentPlayer);
	cfValuesOdCurrentPlayer.resize(1, card_count); // [1(action) X card_count]
	ArrayXX matrixToSubstract = cfValuesOdCurrentPlayer.replicate(actions_count, 1); // [actions X card_count]
	currentPlayerCfValues -= matrixToSubstract; // Substructing sum of CF values over all actions with every action CF value
	return currentPlayerCfValues;
}



