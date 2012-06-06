#include"SDL.h"
#include "SDL_image.h"
#include"SDL_ttf.h"
#include<string>
#include<cctype>
#include<iostream>
#include<vector>
using namespace std;

// to store some possible moves
vector<SDL_Rect> Moves;

//Screen attributes
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 660;
const int SCREEN_BPP = 32;
// checkers on the board
const int SQUARE_WIDTH = 70; 
const int SQUARE_HEIGHT = 70;
// Identifying Pieces From Sprite Sheet and SDL_Rect Clips[12] down below...
const int QUEEN_1 = 0;
const int KING_1 = 1;
const int ROOK_1 = 2;
const int BISHOP_1 = 3;
const int KNIGHT_1 = 4;
const int PAWN_1 = 5;
const int QUEEN_2 = 6;
const int KING_2 = 7;
const int ROOK_2 = 8;
const int BISHOP_2 = 9;
const int KNIGHT_2 = 10;
const int PAWN_2 = 11;

//	Global Screen Surface and Sprite Sheet of Pieces, shimmer to blink the possible moves	
SDL_Surface *screen = NULL;
SDL_Surface *sprite = NULL;
SDL_Surface* shimmer = NULL; 
SDL_Surface* message1 = NULL;
SDL_Surface* message2 = NULL;
SDL_Surface * message3 = NULL;
SDL_Surface * message4 = NULL;
// SDL Event Structure
SDL_Event event;

// pieces to be used in the program;
SDL_Rect clips[12];

// pieces height and width
const int H = 65;
const int W = 65;
// font to be used
TTF_Font *font = NULL;
// the color of the font
SDL_Color textColor = {255,15,60};

class List
{
private:
	class Piece
	{
		friend class List;
		Piece *next;
		int x, y;
		char initials;
		int player;

		Piece( char init, int a, int b, int numPlayer, Piece *nästa = NULL)
		{
			initials = init;
			x = a;
			y = b;
			player = numPlayer;
			next = nästa; //( nästa = swedish for next !)
		}
	};
	Piece *head;					// List header pointer
	int length(Piece*);				// implemented using recursion
	
public:
	List()
		{ head = NULL; }
	~List();
	void operator=(const List&);	// overloaded operator
	bool isEmpty(int,int);
	bool isEnemy(int,int,int,int);   
	bool isLegal(int, int, int, int); //finds out if the move is legal or not
	int getPlayer(int,int);
	char getInitials(int,int);  //returns the initial of the piece
	void appendNode( char, int, int, int);
	//void insertNode(string, string);
	void deleteNode(int,int);
	void make_Move(int,int,int,int);
	int numNodes() { return length(head); }	/* calls private function and returns length of the list */
	bool pawn_move(int,int,int,int);
	bool rook_move(int,int,int,int);
	bool bishop_move(int,int,int,int);
	bool knight_move(int,int,int,int);
	bool king_move(int,int,int,int);
	bool queen_move(int,int,int,int);
	/* Shows all possible moves by calling each peices's move listings...*/
	void get_PossibleMoves(int,int,vector<SDL_Rect>&);
	void possible_kingmoves(int,int,vector<SDL_Rect>&);
	void possible_rookmoves(int,int,vector<SDL_Rect>&);
	void possible_bishopmoves(int,int,vector<SDL_Rect>&);
	void possible_knightmoves(int,int,vector<SDL_Rect>&);
	void possible_pawnmoves(int,int,vector<SDL_Rect>&);
	bool king_Dying(int,int,int,int); /* Check if the king would be in danger */
	bool isChecked( int );//returns true if the player in the argument is Checked by an opponent
	bool handle_events( int );
	void blink_screen( const vector<SDL_Rect>& );
	void showGreetings( int );
	void show_king_saving_moves( int , vector<SDL_Rect>&  );
	
};
// chesboard
List Temp;
List Chess;

SDL_Surface *load_image( std::string filename )
{
	//The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 255, 0, 0 ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void set_clips()
{
	/* Here the positions of different pieces in the Sprite Sheet are being stored in clips array */ 
	for( int i=0; i<12; i++)
	{	
		clips[i].h = H;
		clips[i].w = W;
	}
	
	clips[QUEEN_1].x = 0;
	clips[QUEEN_1].y = 0;

	clips[KING_1].x = W;
	clips[KING_1].y = 0;

	clips[ROOK_1].x = W*2;
	clips[ROOK_1].y = 0;

	clips[BISHOP_1].x = W*3;
	clips[BISHOP_1].y = 0;

	clips[KNIGHT_1].x = W*4;
	clips[KNIGHT_1].y = 0;

	clips[PAWN_1].x = W*5;
	clips[PAWN_1].y = 0;

	clips[QUEEN_2].x = 0;
	clips[QUEEN_2].y = W;

	clips[KING_2].x = W;
	clips[KING_2].y = W;

	clips[ROOK_2].x = W*2;
	clips[ROOK_2].y = W;

	clips[BISHOP_2].x = W*3;
	clips[BISHOP_2].y = W;

	clips[KNIGHT_2].x = W*4;
	clips[KNIGHT_2].y = W;

	clips[PAWN_2].x = W*5;
	clips[PAWN_2].y = W;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
	/* */
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
		return false;
    // Initialize the Fonts Library
	if( TTF_Init() == -1 )
		return false;

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
	
	// sprite has all the pieces we need for the prgram
	sprite = load_image("pieces.png");

	// load a surface to show highlighted area where a piece can move to
	shimmer = load_image("chess_shimmer.png");
	SDL_SetAlpha(shimmer, SDL_SRCALPHA, 90);
	
	// Load the font for use in the program
	font = TTF_OpenFont("Flashit.ttf", 20);
	
	if(!font)
		return false;
	
	if( !sprite)
		return false;
    //If there was an error in setting up the screen
    if( !screen)
		return false;
    
	
	if ( SDL_Flip(screen) == -1 )
		return false;
    //Set the window caption
    SDL_WM_SetCaption( "Simple Two Player Chess Game", NULL );

    //If everything initialized fine
    return true;
}

void display_Pieces()
{
	/* okay this function displays the pieces at its place */
	//5-70
	int size = 70;
	for( int i=0; i<8; i++)
	{	
		for( int j=0; j<8; j++)
		{
			switch( Chess.getInitials(i+1,j+1) )
			{
			case 'R':apply_surface( (j)*size, (i)*size, sprite, screen, &clips[ROOK_1] ); break;				
			case 'r':apply_surface( (j)*size, (i)*size+5, sprite, screen, &clips[ROOK_2] ); break;				
			case 'B':apply_surface( (j)*size, (i)*size, sprite, screen, &clips[BISHOP_1] ); break;
			case 'b':apply_surface( (j)*size, (i)*size+5, sprite, screen, &clips[BISHOP_2] ); break;
			case 'N':apply_surface( (j)*size+5, (i)*size, sprite, screen, &clips[KNIGHT_1] ); break;
			case 'n':apply_surface( (j)*size+5, (i)*size+5, sprite, screen, &clips[KNIGHT_2] ); break;
			case 'q':apply_surface( (j)*size, (i)*size+5, sprite, screen, &clips[QUEEN_2] ); break;
			case 'Q':apply_surface( (j)*size, (i)*size, sprite, screen, &clips[QUEEN_1] ); break;
			case 'K':apply_surface( (j)*size, (i)*size, sprite, screen, &clips[KING_1] ); break;
			case 'k':apply_surface( (j)*size, (i)*size+5, sprite, screen, &clips[KING_2] ); break;
			case 'p':apply_surface( (j)*size+10, (i)*size+5, sprite, screen, &clips[PAWN_2] ); break;
			case 'P':apply_surface( (j)*size+10, (i)*size, sprite, screen, &clips[PAWN_1] ); break;
			default:break;
			}
		}
	}
	
}

void make_Chess_Board()
{
	SDL_Rect box;
	box.w = SQUARE_WIDTH;
	box.h = SQUARE_HEIGHT;
	
	//Fill the screen Magneta!
    SDL_FillRect( screen, &screen->clip_rect, SDL_MapRGB( screen->format, 0, 200, 200 ) );
	
	for( int i=0; i<8; i++)
	{
		for(int j=0; j<8; j++)
		{
			box.x = j*70;
			box.y = i*70;
			if( i==0 || i%2==0)
			{	
				if(j==0 || j%2==0)
					SDL_FillRect(screen, &box, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ));
				else
					SDL_FillRect(screen, &box, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ));
			}
			else
			{
				if( j!=0 && j%2!=0)
					SDL_FillRect(screen, &box, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ));
				else
					SDL_FillRect(screen, &box, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ));
			}
		}
	}
}


int main( int argc, char** argv)
{
	if( init() == false )
    {
        return 1;
    }
	
	/* Place White Pieces on The Board */
	Chess.appendNode( 'R', 1, 1, 1 );
	Chess.appendNode( 'N', 1, 2, 1 );
	Chess.appendNode( 'B', 1, 3, 1 );
	Chess.appendNode( 'K', 1, 4, 1 );
	Chess.appendNode( 'Q', 1, 5, 1 );
	Chess.appendNode( 'B', 1, 6, 1 );
	Chess.appendNode( 'N', 1, 7, 1 );
	Chess.appendNode( 'R', 1, 8, 1 );
	// placing white pawns on its place at the start
	for ( int i = 1; i < 9; i++ )
		Chess.appendNode( 'P', 2, i, 1 );
	
	/* Place Black Pieces on The Board */
	for ( int i = 1; i < 9; i++ )
		Chess.appendNode( 'p', 7, i, 2 );

	Chess.appendNode( 'r', 8, 1, 2 );
	Chess.appendNode( 'n', 8, 2, 2 );
	Chess.appendNode( 'b', 8, 3, 2 );
	Chess.appendNode( 'k', 8, 4, 2 );
	Chess.appendNode( 'q', 8, 5, 2 );
	Chess.appendNode( 'b', 8, 6, 2 );
	Chess.appendNode( 'n', 8, 7, 2 );
	Chess.appendNode( 'r', 8, 8, 2 );
	
	// set the clips: clips are pieces from the sprite sheet 
	set_clips();
	
	// creating surface to render the text
	message1 = TTF_RenderText_Solid(font,"Player 1", textColor);
	message2 = TTF_RenderText_Solid(font,"Player 2", textColor);
	message3 =	TTF_RenderText_Solid(font,"<< P1 KING in Danger", textColor);
	message4 =	TTF_RenderText_Solid(font,"<< P2 KING in Danger", textColor);
	
	if(!message1 || !message2 || !message3 || !message4 )
		return 1;
	
	bool player_1_played = false;
	bool player_2_played = false;
	bool player_1_checked = false;
	bool player_2_checked = false;

	bool checked = false;
	bool quit = false;
	
	while( !quit )
	{
		//While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
			
			// Handle player 1's Moves, until a valid move has been made or the game is over
			if( !player_1_played )
			{
				player_1_checked = Chess.isChecked(1);
				
				// Chess.handle_events returns true if a valid move has been made for the given player..else returns false
				player_1_played = Chess.handle_events(1);
			}

            // Handle player 2's pieces if player one made his move
			else if( player_1_played  )
			{
				player_2_played = false;
				
				// now only player 2 can move his pieces...
				while( !quit && !player_2_played )
				{
					//While there's events to handle
					while( SDL_PollEvent( &event ) )
					{
						player_2_checked = Chess.isChecked(2);
						
						// now only player 2 can move his pieces...
						player_2_played = Chess.handle_events(2);
						if( player_2_played ) player_1_played = false;

						//If escape was pressed
						if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_ESCAPE ) )
							quit = true;
						
						//If the user has Xed out the window
						else if( event.type == SDL_QUIT )
							quit = true;	
					}
					
					// Make Sprites of Black and White Squares for the Chess Game
					make_Chess_Board();
					
					// Displays the Pieces on Their Proper Place on the Board just made above!
					display_Pieces();
					
					// show the active player..
					if( !player_2_played ) apply_surface(600,480, message2, screen);
					if( player_2_checked ) apply_surface(600,300, message4, screen);
					//Update the screen
					if( SDL_Flip( screen ) == -1 )
						return 1;
					
				}	
			}
			//If escape was pressed
			if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_ESCAPE ) )
				quit = true;
			//If the user has Xed out the window
			else if( event.type == SDL_QUIT )
				quit = true;
		}
		
		// Make Sprites of Black and White Squares for the Chess Game
		make_Chess_Board();
		// Displays the Pieces on Their Proper Place on the Board just made above!
		display_Pieces();
		// show the active player..
		if( !player_1_played ) apply_surface(600,75, message1, screen);
		if( player_1_checked ) apply_surface(600,300, message3, screen);
		//else if( !player_2_played ) apply_surface(600,300, message2, screen);
		//Update the screen
		if( SDL_Flip( screen ) == -1 )
			return 1;
	}
	
	return 0;
}

bool List::handle_events( int player_Num )
{
	//ignore garbage calls
	if( player_Num != 1 && player_Num != 2 )
		return false;
	// update the screen
	//SDL_Flip(screen);
	// umm this Vector holds if any, the possible moves of a piece
	vector<SDL_Rect> Regions;
	vector<SDL_Rect> Special;
	bool checked = false;
	
	//The mouse offsets( Source and Target )
    int x_axis = 0, y_axis = 0, x_2 = 0, y_2 = 0;
	int playing = 0;
	
	// whether or not the player is Checked by an opponent
	checked = this->isChecked( player_Num );
	
	//If a mouse button was pressed i'll show possible moves if there are any u noe..
	if( event.type == SDL_MOUSEBUTTONDOWN )
	{
		//If the left mouse button was pressed
		if( event.button.button == SDL_BUTTON_LEFT )
		{
			//Get the mouse offsets
			x_axis = event.button.x / 70;    // each box is of size 70 so need to adjust here,
			y_axis = event.button.y / 70;

			playing = getPlayer(y_axis+1, x_axis+1);
			// if the player is checked the process will be somewhat differenct than usual
			
			if( playing == player_Num )
			{
				// GET THE POSSIBLE MOVES IF ANY FROM THE CLICKED MOUSE OFFSET
				this->show_king_saving_moves( player_Num, Regions );
				if( Regions.empty() )
					showGreetings( player_Num );
				Regions.clear();
				get_PossibleMoves( y_axis+1, x_axis+1, Regions);

				// if either of above two conditions exist do we proceed further else repeat until user cliks in the area where king is			
				if( !Regions.empty() )
				{
					/* If the check has been made show only the valid moves */
					for( int i=0; i<Regions.size(); i++)
					{
						if( !king_Dying(y_axis+1, x_axis+1, Regions[i].y/70+1, Regions[i].x/70+1 ) )
							Special.push_back( Regions[i] );
					}
					
					// We want to show only the valid moves to the user!!!
					Regions = Special;

					// show possible target boxes to the screen and wait for destination...
					blink_screen( Regions );
					
					bool quit = false;
				
					while( !quit )
					{
						//While there's events to handle
						while( SDL_PollEvent( &event ) )
						{
							if( event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT )
							{
								/* at this point x and y offset gives us the target box */
								x_2 = event.button.x / 70;
								y_2 = event.button.y / 70;
					
								if( !king_Dying(y_axis+1, x_axis+1, y_2+1, x_2+1) && isLegal(y_axis+1, x_axis+1, y_2+1, x_2+1) )
								{	
									make_Move(y_axis+1, x_axis+1, y_2+1, x_2+1);
									return true;
								}
								
								// quit the round whatever happens u noe
								quit = true;
							
							}
							//If escape was pressed
							if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_ESCAPE ) )
								quit = true;
							//If the user has X'ed out the window
							else if( event.type == SDL_QUIT )
								quit = true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool List::isChecked( int player_Num )
{
	/*  Returns true if player_Num's King is being checked */
	// first get the position of the king at(x,y)
	// clear all previous record of the moves...
	Moves.clear();

	bool found = false;
	char ch;
	int a = 0, b = 0;

	// find the location of the king first ...
	for( int i=1; !found && i<9; i++)
	{
		for( int j=1; !found && j<9; j++)
		{
			ch = this->getInitials(i,j);	
			if( player_Num == 1 && ch == 'K')
			{	
				a = i; b = j;
				found = true;
			}
			else if( player_Num == 2 && ch == 'k')
			{	
				a = i; b = j;
				found = true;
			}
		}
	}
	
	// At (a,b) on the chess board lies the King of Player_Num
	if( found )
	{
		switch( player_Num )
		{
		case 1:
			// when kings come face to face!!! it should never happen tho...
			if( getInitials(a-1,b-1)=='k'||getInitials(a-1,b)=='k'||getInitials(a-1,b+1)=='k'||getInitials(a,b-1)=='k'||getInitials(a,b+1)=='k'||getInitials(a+1,b-1)=='k'||getInitials(a+1,b)=='k'||getInitials(a+1,b+1)=='k')
				return true;
			
			// find about other pieces than a king himself
			for( int i=1; i<9; i++)
			{
				for( int j=1; j<9; j++)
				{
					// it's very important here not to call when it's king himself to avoid infinite loop
					if( this->getPlayer( i, j) == 2 && this->getInitials(i,j) != 'k')
						this->get_PossibleMoves( i , j , Moves);
				}
			}
			break;
			// second case if it's second player
		case 2:
			// when kings come face to face!!! it should never happen tho...
			if( getInitials(a-1,b-1)=='K'||getInitials(a-1,b)=='K'||getInitials(a-1,b+1)=='K'||getInitials(a,b-1)=='K'||getInitials(a,b+1)=='K'||getInitials(a+1,b-1)=='K'||getInitials(a+1,b)=='K'||getInitials(a+1,b+1)=='K')
				return true;
			// see if king is being killed by other pieces than the king himself
			for( int i=1; i<9; i++)
			{
				for( int j=1; j<9; j++)
				{
					// its a very sensitive code here,,,if calling kings move is not avoided it will loop forever
					if( this->getPlayer( i, j) == 1 && this->getInitials(i,j) != 'K')
						this->get_PossibleMoves( i , j , Moves);
				}
			}
			break;
		default:
			break;
		}

		// now check if the king's position is included in the Moves of enemy piece 
		for( int i=0; i<Moves.size(); i++)
		{
			if( Moves[i].x == (b-1)*70 && Moves[i].y == (a-1)*70 )
				return true;
		}
	}
	return false;
}

bool List::pawn_move(int from_x, int from_y, int to_x, int to_y)
{
	/* pawan can only take one step ahead or sideways when captuaring */
	if(  isEmpty(to_x, to_y) )
	{
		/* move the white pawn */
		if( getPlayer(from_x, from_y) == 1 )
		{	
			if( from_x + 1 == to_x && from_y == to_y )
				return true;
			else if( from_x+2 == to_x && from_y == to_y && from_x == 2 )
				return true;
			else
				return false;
		}
			
		/* move the black pawn */
		else if(getPlayer(from_x, from_y) == 2  )
		{	
			if( from_x - 1 == to_x && from_y == to_y )
				return true;
			else if( from_x - 2 == to_x && from_y == to_y && from_x == 7 )
				return true;
			else 
				return false;
		}

		else
			return false;
	}		
	else if( isEnemy(from_x,from_y,to_x,to_y)  )	/* Captuaring Conditions */
	{	
		/* White Pawan Captures Enemy Piece! */
		if( getPlayer(from_x, from_y) == 1 && from_x + 1 == to_x && (to_y == (from_y + 1) || to_y == (from_y - 1)))
			return true;
			
		/* Black Pawan Captures Enemy Piece! */
		else if( getPlayer(from_x, from_y) == 2 && from_x - 1 == to_x && (to_y == (from_y + 1) || to_y == (from_y - 1)))
			return true;
		else 
			return false;
	}	
	else
		return false;
}

bool List::isLegal( int from_x, int from_y, int to_x, int to_y )
{
	if(from_x == to_x && from_y == to_y)
		return false;
	else if(to_x < 1 || to_x > 8 || to_y < 1 || to_y > 8 )
		return false;
	switch( char(toupper(getInitials(from_x, from_y))) )
	{
		case 'R':
			return rook_move(from_x, from_y, to_x, to_y);
		case 'N':
			return knight_move(from_x, from_y, to_x, to_y);
		case 'B':
			return bishop_move(from_x, from_y, to_x, to_y);
		case 'Q':
			return queen_move(from_x, from_y, to_x, to_y);
		case 'K':
			return king_move(from_x, from_y, to_x, to_y);
		case 'P':
			return pawn_move(from_x, from_y, to_x, to_y);
		default:
			return false;
	}
}
bool List::rook_move(int from_x, int from_y, int to_x, int to_y )
{
	// A Rook can Move Four Direction 
	// Conditions For Right And Left Movement from the Pivot 
	for(int i = 0; i < 7 && from_x == to_x; i++)
	{	
		if( (!isEmpty( from_x, from_y + 1 + i) && to_y > from_y)  || (!isEmpty( from_x, from_y - 1 - i) && to_y < from_y))
		{
			// Checking if it's going to Capture Enemy's Piece 
			if( isEnemy(from_x, from_y, to_x, to_y) && (to_y == from_y + 1 + i || to_y == from_y - 1 - i) )
				return true;
			// Killing Own's Piece or Jumping Over Pieces is Not a Property of a ROOK and is an Illigal Move in doing so
			else
				break;
		}
		// Normal Movement Before any Obstacles 
		else if( (to_y == from_y + 1 + i ||  to_y == from_y - 1 - i) && isEmpty(to_x, to_y) )
			return true;
	}
	
	for(int i = 0; i < 7 && from_y == to_y; i++)
	{
		if( (!isEmpty( from_x + i + 1, from_y ) && to_x > from_x ) || ( !isEmpty( from_x - 1 - i, from_y ) && to_x < from_x) )
		{
			// Checking if it's going to Capture Enemy's Piece 
			if( isEnemy(from_x, from_y, to_x, to_y) && (to_x == from_x + 1 + i || to_x == from_x - 1 - i) )
				return true;
			// Killing Own's Piece or Jumping Over Pieces is Not a Property of a ROOK and is an Illigal Move in doing so
			else
				break;;
		}
		// Checking if the Piece is going to be moved before any obstacles 
		else if(( to_x == from_x + 1 + i ||  to_x == from_x - 1 - i) && isEmpty(to_x, to_y) )
			return true;
	}

	return false; 
}
bool List::knight_move(int from_x, int from_y, int to_x, int to_y )
{
	/* Knight can move fout directions on L shaped paths and it can jump over too */
	if( isEnemy(from_x,from_y,to_x,to_y) || isEmpty(to_x, to_y) )
	{
		if( to_x == from_x + 2 && to_y == from_y + 1 )
			return true;
		else if( to_x == from_x - 2 && to_y == from_y + 1) 
			return true;
		else if( to_x == from_x - 2 && to_y == from_y - 1) 
			return true;
		else if( to_x == from_x + 2 && to_y == from_y - 1) 
			return true;
		else if( to_x == from_x - 1 && to_y == from_y - 2) 
			return true;
		else if( to_x == from_x - 1 && to_y == from_y + 2) 
			return true;
		else if( to_x == from_x + 1 && to_y == from_y - 2) 
			return true;
		else if( to_x == from_x + 1 && to_y == from_y + 2) 
			return true;
		else
			return false;
	}
	else
		return false;
}

bool List::bishop_move(int from_x, int from_y, int to_x, int to_y)
{
	/* Bishop can move four ways diagonally */
	/* upper right path */
	for(int i = 1; (i < 8 && to_x < from_x && to_y > from_y) ; i++)
	{
		if( !isEmpty(from_x - i, from_y + i) )
		{
			/* Better Capture the Enemy or an Illigal Move has been attempted! */
			if( isEnemy(from_x, from_y, from_x - i, from_y + i) && (to_x==from_x-i) && (to_y==from_y+i))
				return true;
			else
				return false;
		}
		else if( from_x - i == to_x  && from_y + i == to_y )
			return true;
	}
	/* Upper left path */
	for(int i = 1; i < 8 && to_x < from_x && to_y < from_y; i++)
	{
		if( !isEmpty(from_x - i, from_y - i) )
		{
			/* Better Capture the Enemy or an Illigal Move has been attempted! */
			if( isEnemy(from_x, from_y, from_x - i , from_y - i) && (to_x==from_x-i) && (to_y==from_y-i) )
				return true;
			else
				return false;
		}
		else if( from_x - i == to_x  && from_y - i == to_y )
			return true;
	}
	/*	Lower left path */
	for(int i = 1; i < 8 && to_x > from_x && to_y < from_y; i++)
	{
		if( !isEmpty(from_x + i, from_y - i) )
		{
			/* Better Capture the Enemy or an Illigal Move has been attempted! */
			if( isEnemy(from_x, from_y, from_x + i, from_y - i) && (to_x==from_x+i) && (to_y==from_y-i) )
				return true;
			else
				return false;
		}
		else if( from_x + i == to_x  && from_y - i == to_y )
			return true;
		
	}
	/*	Lower right path*/
	for(int i = 1; i < 8 && to_x > from_x && to_y > from_y; i++)
	{
		if( !isEmpty(from_x + i, from_y + i) ) 
		{
			/* Better Capture the Enemy or an Illigal Move has been attempted! */
			if( isEnemy(from_x, from_y, from_x + i, from_y + i) && (to_x==from_x+i) && (to_y==from_y+i) )
				return true;
			else
				return false;
		}
		else if( from_x + i == to_x  && from_y + i == to_y )
			return true;
	}
	// If none of the Condition matched Return false!
	return false;
}

bool List::queen_move(int from_x, int from_y, int to_x, int to_y)
{
	/* QUEEN IS LIKE TWO ROOKS AND TWO BISHOPS,..*/
	if(from_x == to_x || from_y == to_y)
		return rook_move( from_x, from_y, to_x,  to_y );	
	else
		return bishop_move( from_x, from_y, to_x,  to_y );	
	
}

bool List::king_move(int from_x, int from_y, int to_x, int to_y)
{
	if( (isEnemy(from_x,from_y,to_x,to_y) || isEmpty(to_x, to_y) ) && !king_Dying(from_x, from_y, to_x, to_y) )
	{
		if(to_x == from_x + 1 && to_y == from_y)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x + 1 && to_y == from_y + 1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x + 1 && to_y == from_y - 1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x  && to_y == from_y-1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x  && to_y == from_y+1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x - 1  && to_y == from_y-1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x - 1 && to_y == from_y+1)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else if(to_x == from_x - 1  && to_y == from_y)
			return true;
			//make_Move(from_x,from_y,to_x,to_y);
		else
			return false;
			
	}
	else
		return false;
}
void List::blink_screen( const vector<SDL_Rect>& Regions)
{
	SDL_Rect area;
	
	for( int i=0; i < Regions.size(); i++)
	{
		area = Regions[i];
		apply_surface(area.x, area.y, shimmer, screen);
	}
	
	SDL_Flip(screen);
	SDL_WM_SetCaption( "Watch Out Possible Moves!", NULL );
}

bool List::king_Dying( int a, int b, int c, int d )
{
	// can the king move to (a,b) without being captured by any of enemy piece huh ???!!
	Temp = Chess;
	// overloaded operator makes it possible to copy chess to Temp...
	int playerNum = 3;
	playerNum = Chess.getPlayer(a,b);
	
	if( playerNum == 1 || playerNum ==2 )
	{	
		Temp.make_Move(a,b,c,d);
		if( playerNum == 1 )
			return Temp.isChecked(1);
		else if( playerNum == 2 )
			return Temp.isChecked(2);
	}
	return true;
}


void List::appendNode( char init, int i, int j, int num_player)
{
	if( head == NULL )
		head = new Piece( init, i, j,  num_player );
	else
	{
		//finding the end
		Piece *nodePtr = head; // used to traverse the list

		while(nodePtr->next != NULL)
			nodePtr = nodePtr->next;

		nodePtr->next = new Piece( init, i, j, num_player);
	}
}

void List::operator=( const List& right )
{
	if( head != NULL )
		head = NULL;
	// get the head of right object ( Chess )
	Piece *nodePtr = right.head;
	
	// now initialize the new object...
	while( nodePtr->next != NULL )
	{
		if( head == NULL)
			head = new Piece( nodePtr->initials, nodePtr->x, nodePtr->y, nodePtr->player);
		else
		{
			Piece *ptr = head;
			while( ptr->next != NULL)
				ptr = ptr->next;
			ptr->next = new Piece( nodePtr->initials, nodePtr->x, nodePtr->y, nodePtr->player);
		}
		nodePtr = nodePtr->next;
	}	
}

void List::deleteNode(int of_x, int of_y)
{
	Piece *nodePtr, *previousNodePtr;
	
	if(!head)
		return;	// do nothing
	if(head->x == of_x && head->y == of_y)
	{
		nodePtr = head;
		head = head->next;
		delete nodePtr;
	}
	else
	{
		nodePtr = head->next;
		previousNodePtr = head;
		
		while(nodePtr)
		{
			if( nodePtr->x == of_x && nodePtr->y == of_y )
				break;
			
			previousNodePtr = nodePtr;
			nodePtr = nodePtr->next;
		}
		if(nodePtr)
		{
			previousNodePtr->next = nodePtr->next;
			delete nodePtr;
		}
	}
}

void List::make_Move(int from_x, int from_y, int to_x, int to_y)
{
	
	bool  found = false;
	Piece * nodePtr;
	
	
	if(!isEmpty( to_x, to_y))
		deleteNode( to_x, to_y );
	nodePtr = head;
	while(nodePtr)
	{
		if(nodePtr->x == from_x && nodePtr->y == from_y)
		{	found = true; break;	}
		else
			nodePtr = nodePtr->next;
	}
	
	if(found)
	{
		nodePtr->x = to_x;
		nodePtr->y = to_y;
	}
}

/*void List::insertNode(double num)
{
	Piece *nodePtr, *previousNodePtr;
	
	if(head == NULL || head->value >= num)
		head = new Piece(num, head);

	else
	{
		nodePtr = head->next;
		previousNodePtr = head;
		// find the insertion point
		while(nodePtr != NULL && nodePtr->value < num)
		{
			previousNodePtr = nodePtr;
			nodePtr = nodePtr->next;
		}
	// create the new node and insert it just before the nodePtr and after previousNodePtr
	previousNodePtr->next = new Piece(num, nodePtr);
	}
}*/


void List::get_PossibleMoves( int a , int b, vector<SDL_Rect>& Regions)
{
	switch( char( toupper( getInitials(a, b) ) ))
	{
	case 'P':
		possible_pawnmoves(a, b, Regions);
		break;
	case 'N':
		possible_knightmoves(a, b, Regions);
		break;
	case 'B':
		possible_bishopmoves(a, b, Regions);
		break;
	case 'K':
		possible_kingmoves(a, b, Regions);
		break;
	case 'Q':
		possible_rookmoves(a, b, Regions);
		possible_bishopmoves(a, b, Regions);
		break;
	case 'R':
		possible_rookmoves(a, b, Regions);
		break;
	default:
		break;
	}
}

void List::possible_rookmoves(int a ,int b, vector<SDL_Rect> &Regions)
{

	if( isLegal( a,b,a,b+1 ) || isLegal( a,b,a,b-1 ) || isLegal( a,b,a-1,b ) || isLegal( a,b,a+1,b)  )
	{	
		SDL_Rect area;
		area.h = 70;
		area.w = 70;
		area.x = (b-1)*70;
		area.y = (a-1)*70;
		Regions.push_back( area );

		for( int i = 0; i < 7; i++)
		{
			if( rook_move( a , b, a-i-1, b) )
			{
				area.x = (b-1)*70;
				area.y = (a-i-2)*70;
				Regions.push_back( area );
			}
			else
				break;
		}

		for( int i = 0; i < 7; i++)
		{
			if( rook_move( a , b, a+i+1, b) )
			{
				area.x = (b-1)*70;// i have to subtract 1 and swap x y to match with output screen ,,,something i didn't see thru in the beginning
				area.y = (a+i)*70;
				Regions.push_back( area );
			}
			else
				break;
		}

		for( int i = 0; i < 7; i++)
		{
			if( rook_move( a , b, a, b-i-1) )
			{
				area.x = (b-i-2)*70;
				area.y = (a-1)*70;
				Regions.push_back( area );
			}	
			else
				break;
		}

		for( int i = 0; i < 7; i++)
		{
			if( rook_move( a , b, a, b+i+1) )
			{
				area.x = (b+i)*70;
				area.y = (a-1)*70;
				Regions.push_back( area );
			}	
			else
				break;
		}
	}
}

void List::possible_bishopmoves(int a ,int b, vector<SDL_Rect> &Regions)
{
	if(isLegal( a,b,a+1,b+1 ) || isLegal( a,b,a-1,b-1 ) || isLegal( a,b,a-1,b+1 ) || isLegal( a,b,a+1,b-1) )
	{	
		SDL_Rect area;
		area.h = 70;
		area.w = 70;
		area.x = (b-1)*70;
		area.y = (a-1)*70;
		Regions.push_back( area );
		
		for( int i = 0; i < 7; i++)
		{
			if( isLegal( a , b, a-i-1, b-i-1) )	
			{
				area.x = (b-i-2)*70;
				area.y = (a-i-2)*70;
				Regions.push_back( area );
			}	
			else
				break;
		}
		for( int i = 0; i < 7; i++)
		{
			if(isLegal( a , b, a+i+1, b-i-1))
			{
				area.x = (b-i-2)*70;
				area.y = (a+i)*70;
				Regions.push_back( area );
			}
			else
				break;
		}
		for( int i = 0; i < 7; i++)
		{
			if(isLegal( a , b, a+i+1, b+i+1))
			{
				area.x = (b+i)*70;
				area.y = (a+i)*70;
				Regions.push_back( area );
			}	
			else
				break;
		}
		for( int i = 0; i < 7; i++)
		{
			if(isLegal( a , b, a-i-1, b+i+1))
			{
				area.x = (b+i)*70;
				area.y = (a-i-2)*70;
				Regions.push_back( area );
			}
			else
				break;
		}
	}
}

void List::possible_knightmoves(int a ,int b, vector<SDL_Rect> &Regions)
{
		if( isLegal(a,b,a-1,b+2) || isLegal(a,b,a-1,b-2) || isLegal(a,b,a+2,b-1) || isLegal(a,b,a-2,b-1) || isLegal(a,b,a+2,b+1) || isLegal(a,b,a-2,b+1) || isLegal(a,b,a+1,b-2) || isLegal(a,b,a+1,b+2))
		{
			SDL_Rect area;
			area.h = 70;
			area.w = 70;
			area.x = (b-1)*70;
			area.y = (a-1)*70;
			Regions.push_back( area );
			
			if( isLegal(a,b,a-1,b+2) )
			{
				area.x = (b+2-1)*70;
				area.y = (a-2)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a-1,b-2) ) 
			{
				area.x = (b-3)*70;
				area.y = (a-2)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a+2,b-1) ) 
			{
				area.x = (b-2)*70;
				area.y = (a+1)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a-2,b-1) ) 
			{
				area.x = (b-2)*70;
				area.y = (a-3)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a+2,b+1) ) 
			{
				area.x = (b)*70;
				area.y = (a+1)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a-2,b+1) ) 
			{
				area.x = (b)*70;
				area.y = (a-3)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a+1,b-2) ) 
			{
				area.x = (b-3)*70;
				area.y = (a)*70;
				Regions.push_back( area );
			}
			if( isLegal(a,b,a+1,b+2) ) 
			{
				area.x = (b+1)*70;
				area.y = (a)*70;
				Regions.push_back( area );
			}
		}
}

void List::possible_pawnmoves(int a, int b, vector<SDL_Rect> &Regions)
{
	if(isLegal(a,b,a+1,b) || isLegal(a,b,a+1,b+1) || isLegal(a,b,a+1,b-1) || isLegal(a,b,a-1,b) || isLegal(a,b,a-1,b-1) || isLegal(a,b,a-1,b+1))
	{
		SDL_Rect area;
		area.h = 70;
		area.w = 70;
		area.x = (b-1)*70;
		area.y = (a-1)*70;
		Regions.push_back( area );

		if( isLegal(a,b,a+2,b) )
		{
			area.x = (b-1)*70;
			area.y = (a)*70;
			Regions.push_back( area );

			area.x = (b-1)*70;
			area.y = (a+1)*70;
			Regions.push_back( area );
		}

		else if( isLegal(a,b,a+1,b) )
		{	
			area.x = (b-1)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}
		
		if( isLegal(a,b,a+1,b+1) )
		{
			area.x = (b)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}

				
		if( isLegal(a,b,a+1,b-1) )
		{
			area.x = (b-2)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}
		
		if( isLegal(a,b,a-2,b) )
		{
			area.x = (b-1)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );

			area.x = (b-1)*70;
			area.y = (a-3)*70;
			Regions.push_back( area );
		}

		else if( isLegal(a,b,a-1,b) )
		{
			area.x = (b-1)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
		
		if( isLegal(a,b,a-1,b-1) )
		{
			area.x = (b-2)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
		
		if( isLegal(a,b,a-1,b+1) )
		{
			area.x = (b)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
	}
}

void List::possible_kingmoves(int a, int b, vector<SDL_Rect> &Regions)
{
	

	if( isLegal( a,b,a+1,b+1 ) || isLegal( a,b,a-1,b-1 ) || isLegal( a,b,a-1,b+1 ) || isLegal( a,b,a+1,b-1) || isLegal( a,b,a,b+1 ) || isLegal( a,b,a,b-1 ) || isLegal( a,b,a-1,b ) || isLegal( a,b,a+1,b)  )
	{
		SDL_Rect area;
		area.h = 70;
		area.w = 70;
		area.x = (b-1)*70;
		area.y = (a-1)*70;
		Regions.push_back( area );

		if( king_move( a , b, a-1, b) )
		{
			area.x = (b-1)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a+1, b))
		{
			area.x = (b-1)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a, b-1))
		{
			area.x = (b-2)*70;
			area.y = (a-1)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a, b+1))
		{
			area.x = (b)*70;
			area.y = (a-1)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a-1, b-1))	
		{
			area.x = (b-2)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a+1, b-1))
		{
			area.x = (b-2)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a+1, b+1))
		{
			area.x = (b)*70;
			area.y = (a)*70;
			Regions.push_back( area );
		}
		if( king_move( a , b, a-1, b+1))
		{
			area.x = (b)*70;
			area.y = (a-2)*70;
			Regions.push_back( area );
		}
	}
}

int List::length(Piece *ptr)
{
	// uses recursion to count number of nodes in the list
	if(ptr == NULL)
		return 0;
	else 
		return 1 + length(ptr->next);
}

char List::getInitials(int lef,int right)
{
	Piece * nodePtr;
	nodePtr = head;
	bool found = false;
	
	if(!head)
	{	cout<<"List is Empty! No Pieces Yet ! Error !\n"; exit(1); }
	
	while(nodePtr)
	{
		if(nodePtr->x == lef & nodePtr->y == right)
		{	
			found = true; 
			break;  
		}
		else
			nodePtr = nodePtr->next;
	}
	if(found)
		return nodePtr->initials;
	else 
		return ' ';
}

bool List::isEmpty(int roW, int coL)
{
	if(roW < 1 || roW > 8 || coL < 1 || coL > 8)
		return false;

	Piece *nodePtr;
	nodePtr =  head;
	if(!head)
		return false;

	while(nodePtr)
	{
		if(nodePtr->x == roW && nodePtr->y == coL)
		{	
			if(nodePtr->initials != ' ')
				return false;
			else
				return true;
		}
		else
			nodePtr = nodePtr->next;
	}
	return true;
}

bool List::isEnemy(int a, int b, int c, int d)
{
	int maker1 = 0, maker2 = 0 ;
	Piece *nodePtr = head;
	if(!head)
		return false;
	while(nodePtr)
	{
		if(nodePtr->x == a && nodePtr->y == b)
			maker1 = nodePtr->player;
		if(nodePtr->x == c && nodePtr->y == d)
			maker2 = nodePtr->player;
		nodePtr = nodePtr->next;
	}
	if(maker1 && maker2)
	{	
		if(maker1 != maker2)
			return true;
		else
			return false;
	}
	else
		return false;
}


int List::getPlayer(int a, int b)
{
	Piece *nodePtr = head;

	while(nodePtr)
	{
		if(nodePtr->x == a && nodePtr->y == b)
			return nodePtr->player;
		else
			nodePtr = nodePtr->next;
	}
	return 3;
}

void List::showGreetings( int a)
{
	switch(a)
	{
	case 1:
		message4 = TTF_RenderText_Solid(font, "Player 2 Won!! Congratulations!!", textColor);
		break;
	case 2:
		message4 = TTF_RenderText_Solid(font, "Player 1 Won!! Congratulations!!", textColor);
		break;
	default:
		break;
	}
	
	
	bool quit = false;
	while(!quit)
	{
		while( SDL_PollEvent( &event ) )
		{
			//If escape was pressed
			if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_ESCAPE ) )
				quit = true;
			//If the user has X'ed out the window
			else if( event.type == SDL_QUIT )
				quit = true;
		}
		apply_surface(150,600,message4,screen);
		SDL_Flip(screen);
	}
	SDL_Quit();
	
}

List::~List()
{
	Piece *nodePtr, *nextNodePtr;
	nodePtr = head;
	while(nodePtr != NULL)
	{
		nextNodePtr = nodePtr->next;
		delete nodePtr;
		nodePtr = nextNodePtr;
	}
}
void List::show_king_saving_moves( int player_Num, vector<SDL_Rect>& Regions )
{
	bool found = false;
	int a = 0, b = 0;
	
	for( int i=1; !found && i<9; i++)
	{
		for( int j=1; !found && j<9; j++)
		{

			if( player_Num == 1 && getInitials(i,j) == 'K')
			{	
				a = i; b = j;
				found = true;
			}
			else if( player_Num == 2 && getInitials(i,j) == 'k')
			{	
				a = i; b = j;
				found = true;
			}
		}
		
	}

	vector<SDL_Rect> saves;
	
	//int player_Num = getPlayer(a,b);
	if( player_Num != 1 && player_Num != 2)
		return; // do nothing

	switch( player_Num )
	{
	case 1:
		for( int i=1; i<9; i++)
		{
			for( int j=1; j<9; j++)
			{
				if( getPlayer( i, j) == 1 )
				{
					Regions.clear();
					get_PossibleMoves( i , j , Regions);
					for( int k=0;  k<Regions.size();  k++ )
					{
						Temp = *this;
						Temp.make_Move(i, j, (Regions[k].y/70+1), (Regions[k].x/70+1));
						if( !Temp.isChecked(1) )
							saves.push_back( Regions[k] );
					}
					
				}
			}
		}
		break;
	case 2:
		for( int i=1; i<9; i++)
		{
			for( int j=1; j<9; j++)
			{
				if( getPlayer( i, j) == 2)
				{
					get_PossibleMoves( i , j , Regions);
					for( int k=0; k<Regions.size(); k++ )
					{
						Temp.make_Move(i, j, Regions[k].y/70+1, Regions[k].x/70+1);
						if( !Temp.isChecked(2) )
							saves.push_back( Regions[k] );
						
						Temp = Chess;
					}
					Regions.clear();
				}
			}
		}
		break;
	default:
		break;
	}
	
	Regions = saves;

}
