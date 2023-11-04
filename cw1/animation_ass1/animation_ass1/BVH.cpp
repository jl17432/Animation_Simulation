
//#include "stdafx.h"
#include <fstream>
#include <string>
#include "BVH.h"
#include <iomanip>


// ƒRƒ“ƒgƒ‰ƒNƒ^
BVH::BVH()
{
	motion = NULL;
	Clear();
}

// ƒRƒ“ƒgƒ‰ƒNƒ^
BVH::BVH( const char * bvh_file_name )
{
	motion = NULL;
	Clear();

	Load( bvh_file_name );
}

// ƒfƒXƒgƒ‰ƒNƒ^
BVH::~BVH()
{
	Clear();
}


// ‘Sî•ñ‚ÌƒNƒŠƒA
void  BVH::Clear()
{
	unsigned int  i;
	for ( i=0; i < channels.size(); i++ )
		delete  channels[ i ];
	for ( i=0; i < joints.size(); i++ )
		delete  joints[ i ];
	if ( motion != NULL )
		delete  motion;

	is_load_success = false;
	
	file_name = "";
	motion_name = "";
	
	num_channel = 0;
	channels.clear();
	joints.clear();
	joint_index.clear();
	
	num_frame = 0;
	interval = 0.0;
	motion = NULL;
}



//
//  BVHƒtƒ@ƒCƒ‹‚Ìƒ[ƒh
//
void  BVH::Load( const char * bvh_file_name )
{
	#define  BUFFER_LENGTH  1024*4

	ifstream  file;
	char      line[ BUFFER_LENGTH ];
	char *    token;
	char      separater[] = " :,\t";
	vector< Joint * >   joint_stack;
	Joint *   joint = NULL;
	Joint *   new_joint = NULL;
	bool      is_site = false;
	double    x, y ,z;
	int       i, j;

	// ‰Šú‰»
	Clear();

	// ƒtƒ@ƒCƒ‹‚Ìî•ñiƒtƒ@ƒCƒ‹–¼E“®ì–¼j‚ÌÝ’è
	file_name = bvh_file_name;
	const char *  mn_first = bvh_file_name;
	const char *  mn_last = bvh_file_name + strlen( bvh_file_name );
	if ( strrchr( bvh_file_name, '\\' ) != NULL )
		mn_first = strrchr( bvh_file_name, '\\' ) + 1;
	else if ( strrchr( bvh_file_name, '/' ) != NULL )
		mn_first = strrchr( bvh_file_name, '/' ) + 1;
	if ( strrchr( bvh_file_name, '.' ) != NULL )
		mn_last = strrchr( bvh_file_name, '.' );
	if ( mn_last < mn_first )
		mn_last = bvh_file_name + strlen( bvh_file_name );
	motion_name.assign( mn_first, mn_last );

	// ƒtƒ@ƒCƒ‹‚ÌƒI[ƒvƒ“
	file.open( bvh_file_name, ios::in );
	if ( file.is_open() == 0 )  return; // ƒtƒ@ƒCƒ‹‚ªŠJ‚¯‚È‚©‚Á‚½‚çI—¹

	// ŠK‘wî•ñ‚Ì“Ç‚Ýž‚Ý
	while ( ! file.eof() )
	{
		// ƒtƒ@ƒCƒ‹‚ÌÅŒã‚Ü‚Å‚«‚Ä‚µ‚Ü‚Á‚½‚çˆÙíI—¹
		if ( file.eof() )  goto bvh_error;

		// ‚Ps“Ç‚Ýž‚ÝAæ“ª‚Ì’PŒê‚ðŽæ“¾
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );

		// ‹ós‚Ìê‡‚ÍŽŸ‚Ìs‚Ö
		if ( token == NULL )  continue;

		// ŠÖßƒuƒƒbƒN‚ÌŠJŽn
		if ( strcmp( token, "{" ) == 0 )
		{
			// Œ»Ý‚ÌŠÖß‚ðƒXƒ^ƒbƒN‚ÉÏ‚Þ
			joint_stack.push_back( joint );
			joint = new_joint;
			continue;
		}
		// ŠÖßƒuƒƒbƒN‚ÌI—¹
		if ( strcmp( token, "}" ) == 0 )
		{
			// Œ»Ý‚ÌŠÖß‚ðƒXƒ^ƒbƒN‚©‚çŽæ‚èo‚·
			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;
			continue;
		}

		// ŠÖßî•ñ‚ÌŠJŽn
		if ( ( strcmp( token, "ROOT" ) == 0 ) ||
		     ( strcmp( token, "JOINT" ) == 0 ) )
		{
			// ŠÖßƒf[ƒ^‚Ìì¬
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back( new_joint );
			if ( joint )
				joint->children.push_back( new_joint );

			// ŠÖß–¼‚Ì“Ç‚Ýž‚Ý
			token = strtok( NULL, "" );
			while ( *token == ' ' )  token ++;
			new_joint->name = token;

			// ƒCƒ“ƒfƒbƒNƒX‚Ö’Ç‰Á
			joint_index[ new_joint->name ] = new_joint;
			continue;
		}

		// ––’[î•ñ‚ÌŠJŽn
		if ( ( strcmp( token, "End" ) == 0 ) )
		{
			new_joint = joint;
			is_site = true;
			continue;
		}

		// ŠÖß‚ÌƒIƒtƒZƒbƒg or ––’[ˆÊ’u‚Ìî•ñ
		if ( strcmp( token, "OFFSET" ) == 0 )
		{
			// À•W’l‚ð“Ç‚Ýž‚Ý
			token = strtok( NULL, separater );
			x = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			y = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			z = token ? atof( token ) : 0.0;
			
			// ŠÖß‚ÌƒIƒtƒZƒbƒg‚ÉÀ•W’l‚ðÝ’è
			if ( is_site )
			{
				joint->has_site = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			}
			else
			// ––’[ˆÊ’u‚ÉÀ•W’l‚ðÝ’è
			{
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;
			}
			continue;
		}

		// ŠÖß‚Ìƒ`ƒƒƒ“ƒlƒ‹î•ñ
		if ( strcmp( token, "CHANNELS" ) == 0 )
		{
			// ƒ`ƒƒƒ“ƒlƒ‹”‚ð“Ç‚Ýž‚Ý
			token = strtok( NULL, separater );
			joint->channels.resize( token ? atoi( token ) : 0 );

			// ƒ`ƒƒƒ“ƒlƒ‹î•ñ‚ð“Ç‚Ýž‚Ý
			for ( i=0; i<joint->channels.size(); i++ )
			{
				// ƒ`ƒƒƒ“ƒlƒ‹‚Ìì¬
				Channel *  channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back( channel );
				joint->channels[ i ] = channel;

				// ƒ`ƒƒƒ“ƒlƒ‹‚ÌŽí—Þ‚Ì”»’è
				token = strtok( NULL, separater );
				if ( strcmp( token, "Xrotation" ) == 0 )
					channel->type = X_ROTATION;
				else if ( strcmp( token, "Yrotation" ) == 0 )
					channel->type = Y_ROTATION;
				else if ( strcmp( token, "Zrotation" ) == 0 )
					channel->type = Z_ROTATION;
				else if ( strcmp( token, "Xposition" ) == 0 )
					channel->type = X_POSITION;
				else if ( strcmp( token, "Yposition" ) == 0 )
					channel->type = Y_POSITION;
				else if ( strcmp( token, "Zposition" ) == 0 )
					channel->type = Z_POSITION;
			}
		}

		// Motionƒf[ƒ^‚ÌƒZƒNƒVƒ‡ƒ“‚ÖˆÚ‚é
		if ( strcmp( token, "MOTION" ) == 0 )
			break;
	}


	// ƒ‚[ƒVƒ‡ƒ“î•ñ‚Ì“Ç‚Ýž‚Ý
	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, separater );
	if ( strcmp( token, "Frames" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	num_frame = atoi( token );

	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, ":" );
	if ( strcmp( token, "Frame Time" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	interval = atof( token );

	num_channel = channels.size();
	motion = new double[ num_frame * num_channel ];

	// ƒ‚[ƒVƒ‡ƒ“ƒf[ƒ^‚Ì“Ç‚Ýž‚Ý
	for ( i=0; i<num_frame; i++ )
	{
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );
		for ( j=0; j<num_channel; j++ )
		{
			if ( token == NULL )
				goto bvh_error;
			motion[ i*num_channel + j ] = atof( token );
			token = strtok( NULL, separater );
		}
	}

	// ƒtƒ@ƒCƒ‹‚ÌƒNƒ[ƒY
	file.close();

	// ƒ[ƒh‚Ì¬Œ÷
	is_load_success = true;

	return;

bvh_error:
	file.close();
}



//
//  BVHœŠiEŽp¨‚Ì•`‰æŠÖ”
//

#include <math.h>
#include "gl/glut.h"


// Žw’èƒtƒŒ[ƒ€‚ÌŽp¨‚ð•`‰æ
void  BVH::RenderFigure( int frame_no, float scale )
{
	// BVHœŠiEŽp¨‚ðŽw’è‚µ‚Ä•`‰æ
	RenderFigure( joints[ 0 ], motion + frame_no * num_channel, scale );
}


// Žw’è‚³‚ê‚½BVHœŠiEŽp¨‚ð•`‰æiƒNƒ‰ƒXŠÖ”j
void  BVH::RenderFigure( const Joint * joint, const double * data, float scale )
{
	glPushMatrix();

	// ƒ‹[ƒgŠÖß‚Ìê‡‚Í•½sˆÚ“®‚ð“K—p
	if ( joint->parent == NULL )
	{
		glTranslatef( data[ 0 ] * scale, data[ 1 ] * scale, data[ 2 ] * scale );
	}
	// ŽqŠÖß‚Ìê‡‚ÍeŠÖß‚©‚ç‚Ì•½sˆÚ“®‚ð“K—p
	else
	{
		glTranslatef( joint->offset[ 0 ] * scale, joint->offset[ 1 ] * scale, joint->offset[ 2 ] * scale );
	}

	// eŠÖß‚©‚ç‚Ì‰ñ“]‚ð“K—piƒ‹[ƒgŠÖß‚Ìê‡‚Íƒ[ƒ‹ƒhÀ•W‚©‚ç‚Ì‰ñ“]j
	int  i;
	for ( i=0; i<joint->channels.size(); i++ )
	{
		Channel *  channel = joint->channels[ i ];
		if ( channel->type == X_ROTATION )
			glRotatef( data[ channel->index ], 1.0f, 0.0f, 0.0f );
		else if ( channel->type == Y_ROTATION )
			glRotatef( data[ channel->index ], 0.0f, 1.0f, 0.0f );
		else if ( channel->type == Z_ROTATION )
			glRotatef( data[ channel->index ], 0.0f, 0.0f, 1.0f );
	}

	// ƒŠƒ“ƒN‚ð•`‰æ
	// ŠÖßÀ•WŒn‚ÌŒ´“_‚©‚ç––’[“_‚Ö‚ÌƒŠƒ“ƒN‚ð•`‰æ
	if ( joint->children.size() == 0 )
	{
		RenderBone( 0.0f, 0.0f, 0.0f, joint->site[ 0 ] * scale, joint->site[ 1 ] * scale, joint->site[ 2 ] * scale );
	}
	// ŠÖßÀ•WŒn‚ÌŒ´“_‚©‚çŽŸ‚ÌŠÖß‚Ö‚ÌÚ‘±ˆÊ’u‚Ö‚ÌƒŠƒ“ƒN‚ð•`‰æ
	if ( joint->children.size() == 1 )
	{
		Joint *  child = joint->children[ 0 ];
		RenderBone( 0.0f, 0.0f, 0.0f, child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
	}
	// ‘SŠÖß‚Ö‚ÌÚ‘±ˆÊ’u‚Ö‚Ì’†S“_‚©‚çŠeŠÖß‚Ö‚ÌÚ‘±ˆÊ’u‚Ö‰~’Œ‚ð•`‰æ
	if ( joint->children.size() > 1 )
	{
		// Œ´“_‚Æ‘SŠÖß‚Ö‚ÌÚ‘±ˆÊ’u‚Ö‚Ì’†S“_‚ðŒvŽZ
		float  center[ 3 ] = { 0.0f, 0.0f, 0.0f };
		for ( i=0; i<joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			center[ 0 ] += child->offset[ 0 ];
			center[ 1 ] += child->offset[ 1 ];
			center[ 2 ] += child->offset[ 2 ];
		}
		center[ 0 ] /= joint->children.size() + 1;
		center[ 1 ] /= joint->children.size() + 1;
		center[ 2 ] /= joint->children.size() + 1;

		// Œ´“_‚©‚ç’†S“_‚Ö‚ÌƒŠƒ“ƒN‚ð•`‰æ
		RenderBone(	0.0f, 0.0f, 0.0f, center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale );

		// ’†S“_‚©‚çŽŸ‚ÌŠÖß‚Ö‚ÌÚ‘±ˆÊ’u‚Ö‚ÌƒŠƒ“ƒN‚ð•`‰æ
		for ( i=0; i<joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			RenderBone(	center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale,
				child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
		}
	}

	// ŽqŠÖß‚É‘Î‚µ‚ÄÄ‹AŒÄ‚Ño‚µ
	for ( i=0; i<joint->children.size(); i++ )
	{
		RenderFigure( joint->children[ i ], data, scale );
	}

	glPopMatrix();
}


// BVHœŠi‚Ì‚P–{‚ÌƒŠƒ“ƒN‚ð•`‰æiƒNƒ‰ƒXŠÖ”j
void  BVH::RenderBone( float x0, float y0, float z0, float x1, float y1, float z1, float bRadius )
{
	// —^‚¦‚ç‚ê‚½‚Q“_‚ðŒ‹‚Ô‰~’Œ‚ð•`‰æ

	// ‰~’Œ‚Ì‚Q’[“_‚Ìî•ñ‚ðŒ´“_EŒü‚«E’·‚³‚Ìî•ñ‚É•ÏŠ·
	GLdouble  dir_x = x1 - x0;
	GLdouble  dir_y = y1 - y0;
	GLdouble  dir_z = z1 - z0;
	GLdouble  bone_length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );

	// •`‰æƒpƒ‰ƒƒ^‚ÌÝ’è
	static GLUquadricObj *  quad_obj = NULL;
	if ( quad_obj == NULL )
		quad_obj = gluNewQuadric();
	gluQuadricDrawStyle( quad_obj, GLU_FILL );
	gluQuadricNormals( quad_obj, GLU_SMOOTH );

	glPushMatrix();

	// •½sˆÚ“®‚ðÝ’è
	glTranslated( x0, y0, z0 );

	// ˆÈ‰ºA‰~’Œ‚Ì‰ñ“]‚ð•\‚·s—ñ‚ðŒvŽZ

	// ‚šŽ²‚ð’PˆÊƒxƒNƒgƒ‹‚É³‹K‰»
	double  length;
	length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );
	if ( length < 0.0001 ) { 
		dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;
	}
	dir_x /= length;  dir_y /= length;  dir_z /= length;

	// Šî€‚Æ‚·‚é‚™Ž²‚ÌŒü‚«‚ðÝ’è
	GLdouble  up_x, up_y, up_z;
	up_x = 0.0;
	up_y = 1.0;
	up_z = 0.0;

	// ‚šŽ²‚Æ‚™Ž²‚ÌŠOÏ‚©‚ç‚˜Ž²‚ÌŒü‚«‚ðŒvŽZ
	double  side_x, side_y, side_z;
	side_x = up_y * dir_z - up_z * dir_y;
	side_y = up_z * dir_x - up_x * dir_z;
	side_z = up_x * dir_y - up_y * dir_x;

	// ‚˜Ž²‚ð’PˆÊƒxƒNƒgƒ‹‚É³‹K‰»
	length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );
	if ( length < 0.0001 ) {
		side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;
	}
	side_x /= length;  side_y /= length;  side_z /= length;

	// ‚šŽ²‚Æ‚˜Ž²‚ÌŠOÏ‚©‚ç‚™Ž²‚ÌŒü‚«‚ðŒvŽZ
	up_x = dir_y * side_z - dir_z * side_y;
	up_y = dir_z * side_x - dir_x * side_z;
	up_z = dir_x * side_y - dir_y * side_x;

	// ‰ñ“]s—ñ‚ðÝ’è
	GLdouble  m[16] = { side_x, side_y, side_z, 0.0,
	                    up_x,   up_y,   up_z,   0.0,
	                    dir_x,  dir_y,  dir_z,  0.0,
	                    0.0,    0.0,    0.0,    1.0 };
	glMultMatrixd( m );

	// ‰~’Œ‚ÌÝ’è
	GLdouble radius= bRadius; // ‰~’Œ‚Ì‘¾‚³
	GLdouble slices = 8.0; // ‰~’Œ‚Ì•úŽËó‚Ì×•ª”iƒfƒtƒHƒ‹ƒg12j
	GLdouble stack = 3.0;  // ‰~’Œ‚Ì—ÖØ‚è‚Ì×•ª”iƒfƒtƒHƒ‹ƒg‚Pj

	// ‰~’Œ‚ð•`‰æ
	gluCylinder( quad_obj, radius, radius, bone_length, slices, stack ); 

	glPopMatrix();
}


void BVH::writeBVH(BVH * bvhPointer) {

	//passing nullptr, report and return
	if (!bvhPointer)
	{
		cout << "no BVH file to write" << endl;
		return;
	}

	//ouput file
	ofstream output_file;

	//generate new file name
	string output_file_name = bvhPointer->file_name;
	std::cout << output_file_name << endl;
	output_file_name = output_file_name.substr(0, output_file_name.size() - 4);
	output_file_name += "_new.bvh";

	//open outputfile
	output_file.open(output_file_name, ios::out | ios::in | ios::trunc);
	output_file << std::fixed << std::setprecision(6);

	//title: hierarchy
	output_file << "HIERARCHY" << endl;


	//get the root
	Joint* root = bvhPointer->joints[0];
	while (root->parent) {
		root = root->parent;
	}
	std::unordered_map<BVH::ChannelEnum, string> channel_dictionary = BVH::construct_channel_name_dic();
	//write joints
	BVH::recurrsiveBVHwriter(root, output_file, 0, bvhPointer->BVH::GetNumJoint(), channel_dictionary);
	//write motions
	output_file << "MOTION" << endl;
	output_file << "Frames: " << bvhPointer->num_frame << endl;
	output_file << "Frame Time: " << bvhPointer->interval << endl;

	//write motion data
	for (int i = 0; i < bvhPointer->num_frame; i++)
	{
		for (int j = 0; j < bvhPointer->num_channel; j++)
		{
			//compute the current motion data
			output_file << bvhPointer->motion[i * bvhPointer->GetNumChannel() + j] << " ";
		}
		output_file << endl;
	}

	// finished writing file
	cout << "finish writing file: " << output_file_name << endl;

}


void BVH::recurrsiveBVHwriter(BVH::Joint * joint, ofstream& output_file, int tab_counts, int no_of_joints, std::unordered_map<BVH::ChannelEnum, string>& dictionary)
{
	//generate the tab string
	string tab_string = "";
	for (int i = 0; i < tab_counts; i++)
	{
		tab_string += "\t";
	}
	//writing a root
	if (!joint->parent) 
	{
		output_file << tab_string << "ROOT " << joint->name << endl;
	}
	//writing a joint
	else
	{
		output_file << tab_string << "JOINT " << joint->name << endl;
	}

	output_file << tab_string << "{" << endl;
	//increment count for tabs
	tab_counts++;
	tab_string += "\t";
	output_file << tab_string << "OFFSET" << " " << joint->offset[0] << " " << joint->offset[1] << " " << joint->offset[2] << endl;
	output_file << tab_string << "CHANNELS" << " " << joint->channels.size();
	//writing channels
	for (auto item : joint->channels)
	{
		output_file << " " << dictionary[item->type];
	}
	output_file << endl;

	//writing the endsite
	if (joint->has_site)
	{
		output_file << tab_string << "End Site" << endl;
		output_file << tab_string << "{" << endl;
		output_file << tab_string << '\t' << "OFFSET " << joint->site[0] << " " << joint->site[1] << " " << joint->site[2] << endl;
		output_file << tab_string << "}" << endl;
	}
	//the recursive part
	for (auto child : joint->children)
	{
		BVH::recurrsiveBVHwriter(child, output_file, tab_counts, no_of_joints, dictionary);
	}
	//write the right bracket
	tab_string.pop_back();
	output_file << tab_string << "}" << endl; 
	

	return;
}


//generate a dictionary for lookup the pairing string of channel type
//this will be used when writing data into the output file
std::unordered_map<BVH::ChannelEnum, string> BVH::construct_channel_name_dic()
{
	std::unordered_map<BVH::ChannelEnum, string> return_map;
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::X_ROTATION, "Xrotation"));
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::Y_ROTATION, "Yrotation"));
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::Z_ROTATION, "Zrotation"));
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::X_POSITION, "Xposition"));
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::Y_POSITION, "Yposition"));
	return_map.insert(pair<BVH::ChannelEnum, string>(BVH::ChannelEnum::Z_POSITION, "Zposition"));
	return return_map;
}



// End of BVH.cpp
