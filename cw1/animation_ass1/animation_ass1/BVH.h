/**
***  BVH“®ìƒtƒ@ƒCƒ‹‚Ì“Ç‚Ýž‚ÝE•`‰æƒNƒ‰ƒX
***  Copyright (c) 2004-2007, Masaki OSHITA (www.oshita-lab.org)
**/

#pragma warning(disable: 4018)
#ifndef  _BVH_H_
#define  _BVH_H_


#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <unordered_map>


using namespace  std;



//
//  BVHŒ`Ž®‚Ìƒ‚[ƒVƒ‡ƒ“ƒf[ƒ^
//
class  BVH
{
  public:
	/*  “à•”—p\‘¢‘Ì  */

	// ƒ`ƒƒƒ“ƒlƒ‹‚ÌŽí—Þ
	enum  ChannelEnum
	{
		X_ROTATION, Y_ROTATION, Z_ROTATION,
		X_POSITION, Y_POSITION, Z_POSITION
	};
	struct  Joint;

	// ƒ`ƒƒƒ“ƒlƒ‹î•ñ
	struct  Channel
	{
		// ‘Î‰žŠÖß
		Joint *              joint;
		
		// ƒ`ƒƒƒ“ƒlƒ‹‚ÌŽí—Þ
		ChannelEnum          type;

		// ƒ`ƒƒƒ“ƒlƒ‹”Ô†
		int                  index;
	};

	// ŠÖßî•ñ
	struct  Joint
	{
		// ŠÖß–¼
		string               name;
		// ŠÖß”Ô†
		int                  index;

		// ŠÖßŠK‘wieŠÖßj
		Joint *              parent;
		// ŠÖßŠK‘wiŽqŠÖßj
		vector< Joint * >    children;

		// Ú‘±ˆÊ’u
		double               offset[3];

		// ––’[ˆÊ’uî•ñ‚ðŽ‚Â‚©‚Ç‚¤‚©‚Ìƒtƒ‰ƒO
		bool                 has_site;
		// ––’[ˆÊ’u
		double               site[3];

		// ‰ñ“]Ž²
		vector< Channel * >  channels;
	};


  public:
	// ƒ[ƒh‚ª¬Œ÷‚µ‚½‚©‚Ç‚¤‚©‚Ìƒtƒ‰ƒO
	bool                     is_load_success;

	/*  ƒtƒ@ƒCƒ‹‚Ìî•ñ  */
	string                   file_name;   // ƒtƒ@ƒCƒ‹–¼
	string                   motion_name; // “®ì–¼

	/*  ŠK‘w\‘¢‚Ìî•ñ  */
	int                      num_channel; // ƒ`ƒƒƒ“ƒlƒ‹”
	vector< Channel * >      channels;    // ƒ`ƒƒƒ“ƒlƒ‹î•ñ [ƒ`ƒƒƒ“ƒlƒ‹”Ô†]
	vector< Joint * >        joints;      // ŠÖßî•ñ [ƒp[ƒc”Ô†]
	map< string, Joint * >   joint_index; // ŠÖß–¼‚©‚çŠÖßî•ñ‚Ö‚ÌƒCƒ“ƒfƒbƒNƒX

	/*  ƒ‚[ƒVƒ‡ƒ“ƒf[ƒ^‚Ìî•ñ  */
	int                      num_frame;   // ƒtƒŒ[ƒ€”
	double                   interval;    // ƒtƒŒ[ƒ€ŠÔ‚ÌŽžŠÔŠÔŠu
	double *                 motion;      // [ƒtƒŒ[ƒ€”Ô†][ƒ`ƒƒƒ“ƒlƒ‹”Ô†]




  public:
	// ƒRƒ“ƒXƒgƒ‰ƒNƒ^EƒfƒXƒgƒ‰ƒNƒ^
	BVH();
	BVH( const char * bvh_file_name );
	~BVH();

	// ‘Sî•ñ‚ÌƒNƒŠƒA
	void  Clear();

	// BVHƒtƒ@ƒCƒ‹‚Ìƒ[ƒh
	void  Load( const char * bvh_file_name );

  public:
	/*  ƒf[ƒ^ƒAƒNƒZƒXŠÖ”  */

	// ƒ[ƒh‚ª¬Œ÷‚µ‚½‚©‚Ç‚¤‚©‚ðŽæ“¾
	bool  IsLoadSuccess() const { return is_load_success; }

	// ƒtƒ@ƒCƒ‹‚Ìî•ñ‚ÌŽæ“¾
	const string &  GetFileName() const { return file_name; }
	const string &  GetMotionName() const { return motion_name; }

	// ŠK‘w\‘¢‚Ìî•ñ‚ÌŽæ“¾
	const int       GetNumJoint() const { return  joints.size(); }
	const Joint *   GetJoint( int no ) const { return  joints[no]; }
	const int       GetNumChannel() const { return  channels.size(); }
	const Channel * GetChannel( int no ) const { return  channels[no]; }

	const Joint *   GetJoint( const string & j ) const  {
		map< string, Joint * >::const_iterator  i = joint_index.find( j );
		return  ( i != joint_index.end() ) ? (*i).second : NULL; }
	const Joint *   GetJoint( const char * j ) const  {
		map< string, Joint * >::const_iterator  i = joint_index.find( j );
		return  ( i != joint_index.end() ) ? (*i).second : NULL; }

	// ƒ‚[ƒVƒ‡ƒ“ƒf[ƒ^‚Ìî•ñ‚ÌŽæ“¾
	int     GetNumFrame() const { return  num_frame; }
	double  GetInterval() const { return  interval; }
	double  GetMotion( int f, int c ) const { return  motion[ f*num_channel + c ]; }

	// ƒ‚[ƒVƒ‡ƒ“ƒf[ƒ^‚Ìî•ñ‚Ì•ÏX
	void  SetMotion( int f, int c, double v ) { motion[ f*num_channel + c ] = v; }

  public:
	/*  Žp¨‚Ì•`‰æŠÖ”  */
	
	// Žw’èƒtƒŒ[ƒ€‚ÌŽp¨‚ð•`‰æ
	void  RenderFigure( int frame_no, float scale = 1.0f );

	// Žw’è‚³‚ê‚½BVHœŠiEŽp¨‚ð•`‰æiƒNƒ‰ƒXŠÖ”j
	static void  RenderFigure( const Joint * root, const double * data, float scale = 1.0f );

	// BVHœŠi‚Ì‚P–{‚ÌƒŠƒ“ƒN‚ð•`‰æiƒNƒ‰ƒXŠÖ”j
	static void  RenderBone( float x0, float y0, float z0, float x1, float y1, float z1, float bRadius = 0.1 );


	//write data into a bvh file
	void writeBVH(BVH* bvhPointer);

	//recurrsively write data into bvh file
	void recurrsiveBVHwriter(BVH::Joint* joint, ofstream& output_file, int tab_counts, int no_of_joints, std::unordered_map<BVH::ChannelEnum, string>& dictionary);

	//generate a dictionary pairing channel types and its string
	std::unordered_map<BVH::ChannelEnum, string> construct_channel_name_dic();
};



#endif // _BVH_H_
