//
//  main.cpp
//  2048
//
//  Created by Max Snijders on 29/10/2018.
//  Copyright © 2018 Max Snijders. All rights reserved.
//

#include <iostream>
#include <stdlib.h>     /* calloc, exit, free */
#include <math.h>       /* pow */

typedef enum move_direction {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
} move_direction;

class Board{
public:
    uint16_t* board;
    uint16_t* memory_board;
    uint16_t* move_change_memory_board;
    
    // returns true with a ten percent probability
    bool get_true_with_ten_percent_chance(){
        return (std::rand() % 10 == 0);
    }
    
    uint8_t get_one_with_ninety_percent_chance_or_two(){
        if( get_true_with_ten_percent_chance() ) return 2;
        return 1;
    }
    
    void board_index_assign(uint16_t* board, uint8_t i, uint8_t j, uint16_t value){
        *(board + (i + 4 * j)) = value;
    }
    
    uint16_t const board_index_get(uint16_t* board, uint8_t i, uint8_t j){
        return *(board + (i + 4 * j));
    }
    
    uint32_t score;
    
    Board(){
        score = 0;
        board = (uint16_t*) calloc(16, sizeof(uint16_t));
        memory_board = (uint16_t*) calloc(16, sizeof(uint16_t));
        move_change_memory_board = (uint16_t*) calloc(16, sizeof(uint16_t));
    }
    
    void start_game(){
        // Initialize the game!
        for(uint8_t i = 0; i < 3; i++){
            uint8_t pos = (uint8_t) std::rand() % 16;
            board_index_assign(board, pos / 4, pos % 4, get_one_with_ninety_percent_chance_or_two());
        }
    }
    
    void print(uint16_t* board){
        for(uint8_t i = 0; i < 4; i++){
            for(uint8_t j = 0; j < 4; j++){
                std::cout << (int) board_index_get(board, i, j) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    void print(){
        print(board);
    }
    
    move_direction complementary_direction(move_direction direction){
        switch(direction){
            case UP: return UP;
            case DOWN: return DOWN;
            case LEFT: return RIGHT;
            case RIGHT: return LEFT;
        }
    }
    
    // performs rotation by building memory_board out of board and then switching pointers
    void rotate_board_so_that_direction_is_up(move_direction direction){
        switch(direction){
            case DOWN: {
                for(uint8_t i = 0; i < 4; i++){
                    for(uint8_t j = 0; j < 4; j++){
                        board_index_assign(memory_board, 3-i, 3-j, board_index_get(board, i, j));
                    }
                }
                uint16_t* temp = memory_board;
                memory_board = board;
                board = temp;
                break;
            }
            case LEFT: {
                for(uint8_t i = 0; i < 4; i++){
                    for(uint8_t j = 0; j < 4; j++){
                        board_index_assign(memory_board, 3-j, i, board_index_get(board, i, j));
                    }
                }
                uint16_t* temp = memory_board;
                memory_board = board;
                board = temp;
                break;
            }
            case RIGHT: {
                for(uint8_t i = 0; i < 4; i++){
                    for(uint8_t j = 0; j < 4; j++){
                        board_index_assign(memory_board, j, 3-i, board_index_get(board, i, j));
                    }
                }
                uint16_t* temp = memory_board;
                memory_board = board;
                board = temp;
                break;
            }
            case UP: {
                break;
            }
        }
    }
    
    // in-place on member board
    void flush_up(){
        for(uint8_t j = 0; j < 4; j++){ // For each column perform the same operation...
            
            for(uint8_t i = 1; i < 4; i++){ // For each row from the second one onwards ...
                
                uint8_t steps_needed = 0;
                for(uint8_t k = 1; k <= i; k++){
                    if(board_index_get(board, i-k, j) != 0){ // check the value at k positions above our tile
                        steps_needed = k-1;
                        break; // We need k-1 steps since k steps give us a nonzero tile
                    }
                    steps_needed = k;
                }
                
                // Move ourselves and everything below us up with steps_needed spaces
                uint16_t original_value = board_index_get(board, i, j);
                if(original_value > 0){
                    board_index_assign(board, i-steps_needed, j, original_value);
                    if(steps_needed >= 1) board_index_assign(board, i, j, 0);
                }
            }
        }
    }
    
    // Modifies board in-place
    void merge_up(){
        for(uint8_t j = 0; j < 4; j++){
            for(uint8_t i = 0; i < 3; i++){
                // If we have the same value as the tile above, add the values
                uint16_t us = board_index_get(board, i, j);
                uint16_t them = board_index_get(board, i+1, j);
                if(us > 0 and us == them){
                    score += pow(2, us + 1);
                    board_index_assign(board, i, j, us + 1);
                    board_index_assign(board, i+1, j, 0);
                }
            }
        }
    }
    
    // returns TRUE if the move was legal
    bool move_without_new_tile(move_direction direction){
        // copy the board to our memory buffer so that we can compare the new board with it
        // to check if the move did anything (And thus if it was legal)
        memcpy(move_change_memory_board, board, sizeof(uint16_t) * 16);
        
        // rotate the board so that direction is UP
        rotate_board_so_that_direction_is_up(complementary_direction(direction));
        
        // Perform board update (always moving UP)
        flush_up();
        merge_up();
        flush_up();
        
        // rotate the board to restore its original orientation
        rotate_board_so_that_direction_is_up(direction);
        
        bool move_legal = false;
        for(uint8_t i = 0; i < 4; i++){
            for(uint8_t j = 0; j < 4; j++){
                if(board_index_get(board, i, j) != board_index_get(move_change_memory_board, i, j)){
                    move_legal = true;
                    goto change_checked;
                }
            }
        }
    change_checked:
        return move_legal;
    }
    
    void place_tile_after_move(){
        uint16_t number_of_zeros = 0;
        for(uint8_t i = 0; i < 4; i++){
            for(uint8_t j = 0; j < 4; j++){
                if(board_index_get(board, i, j) == 0){
                    number_of_zeros++;
                }
            }
        }
        
        uint16_t position = std::rand() % number_of_zeros;
        number_of_zeros = 0;
        for(uint8_t i = 0; i < 4; i++){
            for(uint8_t j = 0; j < 4; j++){
                if(board_index_get(board, i, j) == 0){
                    if(number_of_zeros == position) board_index_assign(board, i, j, get_one_with_ninety_percent_chance_or_two());
                    number_of_zeros++;
                }
            }
        }
    }
    
    bool move(move_direction direction){
        bool move_legal = move_without_new_tile(direction);
        if(!move_legal) return false;
        
        // The move was legal, place a new tile
        place_tile_after_move();
        
        return true; // We had already checked that the move was legal.
    }
    
    // Copy board state and score from other board
    void copy_state(Board other){
        score = other.score;
        
        for(uint8_t i = 0; i < 4; i++){
            for(uint8_t j = 0; j < 4; j++){
                board_index_assign(board, i, j, board_index_get(other.board, i, j));
            }
        }
    }
};

typedef enum strategy {
    HIGHEST_AVERAGE_SCORE,
    HIGHEST_ATTAINED_SCORE,
} strategy;

class Player{
private:
    Board move_board;
    Board random_board;
    
public:
    Board master_board;
    uint64_t games_per_legal_move_played;
    
    Player(uint64_t _games_per_legal_move_played){
        // Initialize the master_board
        master_board.start_game();
        games_per_legal_move_played = _games_per_legal_move_played;
    }
    
    bool move(strategy strategy){
        uint64_t sum_of_scores_for_direction[4];
        uint64_t max_score_for_direction[4];
        
        bool any_move_was_legal = false;
        for(uint8_t dir_index = 0; dir_index < 4; dir_index++){
            sum_of_scores_for_direction[dir_index] = 0;
            max_score_for_direction[dir_index] = 0;
            move_direction move = static_cast<move_direction>(dir_index);
            
            // Copy the current_board to move
            move_board.copy_state( master_board );
            
            // Perform the move on that board
            bool move_legal = move_board.move_without_new_tile(move);
            
            // If the move wasn't legal, we don't do anything and keep the score at zero.
            if(!move_legal){
                continue; // Don't play any games here.
            }
            any_move_was_legal = true;
            
            
            // Now, for games_per_legal_move_played different games...
            for(uint64_t game_counter = 0; game_counter < games_per_legal_move_played; game_counter++){
                // Play a random game, get the final score from it.
                // Copy the board over to random_board.
                random_board.copy_state( move_board );
                
                // Finish the move for this direction by placing a tile
                random_board.place_tile_after_move();
                
                // And play the game randomly.
                uint32_t score = play_random_game();
                sum_of_scores_for_direction[dir_index] += score;
                
                if(score > max_score_for_direction[dir_index]) max_score_for_direction[dir_index] = score;
            }
            
        }
        
        move_direction taken_move;
        if(strategy == HIGHEST_AVERAGE_SCORE){
            uint64_t max_score = 0;
            uint8_t max_score_index = 0;
            for(uint8_t i = 0; i < 4; i++){
                if(sum_of_scores_for_direction[i] >= max_score){
                    max_score = sum_of_scores_for_direction[i];
                    max_score_index = i;
                }
            }
            taken_move = static_cast<move_direction>(max_score_index);
        } else {
            uint64_t max_score = 0;
            uint8_t max_score_index = 0;
            for(uint8_t i = 0; i < 4; i++){
                if(max_score_for_direction[i] >= max_score){
                    max_score = max_score_for_direction[i];
                    max_score_index = i;
                }
            }
            taken_move = static_cast<move_direction>(max_score_index);
        }
        
        // We have found the move, do it.
        master_board.move(taken_move);
        
        return any_move_was_legal;
    }
    
    // Plays a random game for the random_board and returns its final score
    uint32_t play_random_game(){

        // While the board lives, we play moves
        uint64_t move_counter = 0;
        while(true){
            bool moves_available[4];
            moves_available[0] = moves_available[1] = moves_available[2] = moves_available[3] = true;
            uint8_t moves_available_count = 4;
            
            // Every iteration of this wile loop is a move
            while(true){
                
                // While we haven't done amove yet we get a random move
                uint8_t move = std::rand() % moves_available_count;
                
                // Get the move taken by the random number generator
                move_direction move_direction = UP;
                uint8_t moves_available_counter = 0;
                for(uint8_t i = 0; i < 4; i++){
                    if(moves_available[i]){
                        if(moves_available_counter == move){
                            // We have the move.
                            move_direction = static_cast<enum move_direction>(i);
                        }
                        moves_available_counter++;
                    }
                }
                
                // Try the move
                bool move_legal = random_board.move(move_direction);
                
                // if the move was illegal we mark it as such, and if there are no moves left we terminate.
                if(!move_legal){
                    moves_available[ static_cast<uint32_t>(move_direction) ] = false;
                    moves_available_count--;
                    
                    if(moves_available_count == 0) return random_board.score;
                } else {
                    break; // We break out of the inner loop since the move was legal
                }
            
            }
            
            move_counter++;
        }
       
    }
};

int main(int argc, const char * argv[]) {
    std::srand(134325);
    int move_counter = 0;

//    // Normal play
//    Player p(1000);
//    p.master_board.print();
//
//    bool move_possible = true;
//    while(move_possible){
//        move_counter++;
//        move_possible = p.move(HIGHEST_AVERAGE_SCORE);
////         move_possible = p.move(HIGHEST_ATTAINED_SCORE);
//        std::cout << "Moves: " << move_counter << std::endl;
//        p.master_board.print();
//    }
    
    // Adaptive play
    Player p(1000);
    p.master_board.print();

    bool move_possible = true;
    while(move_possible){
        move_counter++;
        p.games_per_legal_move_played += 10;

         move_possible = p.move(HIGHEST_AVERAGE_SCORE);
        if(move_counter % 1 == 0) {
            std::cout << "Moves: " << move_counter << std::endl;
            p.master_board.print();
        }
    }
    
    std::cout << "Final state: " << std::endl;
    std::cout << "Moves: " << move_counter << std::endl;
    p.master_board.print();
    
    
    
    
    return 0;
}
