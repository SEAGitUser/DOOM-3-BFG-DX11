/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

cbuffer BuiltinConstants : register(b0)
{
    float4 rpScreenCorrectionFactor	;
    float4 rpWindowCoord			;
    float4 rpDiffuseModifier		;
    float4 rpSpecularModifier		;
                                    ;
    float4 rpLocalLightOrigin		;
    float4 rpLocalViewOrigin		;
                                    ;
    float4 rpLightProjectionS		;
    float4 rpLightProjectionT		;
    float4 rpLightProjectionQ		;
    float4 rpLightFalloffS			;
                                    ;
    float4 rpBumpMatrixS			;
    float4 rpBumpMatrixT			;
                                    ;
    float4 rpDiffuseMatrixS			;
    float4 rpDiffuseMatrixT			;
                                    ;
    float4 rpSpecularMatrixS		;
    float4 rpSpecularMatrixT		;
                                    ;
    float4 rpVertexColorModulate	;
    float4 rpVertexColorAdd			;
                                    ;
    float4 rpColor					;
    float4 rpViewOrigin				;
    float4 rpGlobalEyePos			;
                                    ;
    float4 rpMVPmatrixX				;
    float4 rpMVPmatrixY				;
    float4 rpMVPmatrixZ				;
    float4 rpMVPmatrixW				;
                                    ;
    float4 rpModelMatrixX			;
    float4 rpModelMatrixY			;
    float4 rpModelMatrixZ			;
    float4 rpModelMatrixW			;
                                    ;
    float4 rpProjectionMatrixX		;
    float4 rpProjectionMatrixY		;
    float4 rpProjectionMatrixZ		;
    float4 rpProjectionMatrixW		;
                                    ;
    float4 rpModelViewMatrixX		;
    float4 rpModelViewMatrixY		;
    float4 rpModelViewMatrixZ		;
    float4 rpModelViewMatrixW		;
                                    ;
    float4 rpTextureMatrixS			;
    float4 rpTextureMatrixT			;
                                    ;
    float4 rpTexGen0S				;
    float4 rpTexGen0T				;
    float4 rpTexGen0Q				;
    float4 rpTexGen0Enabled			;
                                    ;
    float4 rpTexGen1S				;
    float4 rpTexGen1T				;
    float4 rpTexGen1Q				;
    float4 rpTexGen1Enabled			;
                                    ;
    float4 rpWobbleSkyX				;
    float4 rpWobbleSkyY				;
    float4 rpWobbleSkyZ				;
                                    ;
    float4 rpOverbright				;
    float4 rpEnableSkinning			;
    float4 rpAlphaTest				;

    uint4  rpJointOffset            ;
};

cbuffer UserConstants : register(b1)
{
    float4 rpUser0;
    float4 rpUser1;
    float4 rpUser2;
    float4 rpUser3;
    float4 rpUser4;
    float4 rpUser5;
    float4 rpUser6;
    float4 rpUser7;
};

cbuffer JointBuffer : register(b2)
{
    float4 matrices[408];
};

static float dot2( float2 a, float2 b ) { return dot( a, b ); }
static float dot3( float3 a, float3 b ) { return dot( a, b ); }
static float dot3( float3 a, float4 b ) { return dot( a, b.xyz ); }
static float dot3( float4 a, float3 b ) { return dot( a.xyz, b ); }
static float dot3( float4 a, float4 b ) { return dot( a.xyz, b.xyz ); }
static float dot4( float4 a, float4 b ) { return dot( a, b ); }
static float dot4( float2 a, float4 b ) { return dot( float4( a, 0, 1 ), b ); }

// ----------------------
// YCoCg Color Conversion
// ----------------------
static const float4 matrixRGB1toCoCg1YX = float4(  0.50,  0.0, -0.50, 0.50196078 );	// Co
static const float4 matrixRGB1toCoCg1YY = float4( -0.25,  0.5, -0.25, 0.50196078 );	// Cg
static const float4 matrixRGB1toCoCg1YZ = float4(  0.0,   0.0,  0.0,  1.0 );			// 1.0
static const float4 matrixRGB1toCoCg1YW = float4(  0.25,  0.5,  0.25, 0.0 );			// Y

static const float4 matrixCoCg1YtoRGB1X = float4(  1.0, -1.0,  0.0,        1.0 );
static const float4 matrixCoCg1YtoRGB1Y = float4(  0.0,  1.0, -0.50196078, 1.0 ); // -0.5 * 256.0 / 255.0
static const float4 matrixCoCg1YtoRGB1Z = float4( -1.0, -1.0,  1.00392156, 1.0 ); // +1.0 * 256.0 / 255.0

static half3 ConvertYCoCgToRGB( float4 YCoCg ) {
	half3 rgbColor;

	YCoCg.z = ( YCoCg.z * 31.875 ) + 1.0;			//z = z * 255.0/8.0 + 1.0
	YCoCg.z = 1.0 / YCoCg.z;
	YCoCg.xy *= YCoCg.z;
	rgbColor.x = dot4( YCoCg, matrixCoCg1YtoRGB1X );
	rgbColor.y = dot4( YCoCg, matrixCoCg1YtoRGB1Y );
	rgbColor.z = dot4( YCoCg, matrixCoCg1YtoRGB1Z );
	return rgbColor;
}

static float2 CenterScale( float2 inTC, float2 centerScale ) {
	float scaleX = centerScale.x;
	float scaleY = centerScale.y;
	float4 tc0 = float4( scaleX, 0, 0, 0.5 - ( 0.5f * scaleX ) );
	float4 tc1 = float4( 0, scaleY, 0, 0.5 - ( 0.5f * scaleY ) );

	float2 finalTC;
	finalTC.x = dot4( inTC, tc0 );
	finalTC.y = dot4( inTC, tc1 );
	return finalTC;
}

static float2 Rotate2D( float2 inTC, float2 cs ) {
	float sinValue = cs.y;
	float cosValue = cs.x;

	float4 tc0 = float4( cosValue, -sinValue, 0, ( -0.5f * cosValue ) + ( 0.5f * sinValue ) + 0.5f );
	float4 tc1 = float4( sinValue, cosValue, 0, ( -0.5f * sinValue ) + ( -0.5f * cosValue ) + 0.5f );

	float2 finalTC;
	finalTC.x = dot4( inTC, tc0 );
	finalTC.y = dot4( inTC, tc1 );
	return finalTC;
}

// better noise function available at https://github.com/ashima/webgl-noise
float rand( float2 co ) {
    return frac( sin( dot( co.xy, float2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}

float4 _half4( half x ) { return x.xxxx; }
float3 _half3( half x ) { return x.xxx; }
float2 _half2( half x ) { return x.xx; }

float4 _float4( float x ) { return x.xxxx; }
float3 _float3( float x ) { return x.xxx; }
float2 _float2( float x ) { return x.xx; }

#define VPOS WPOS
static float4 idtex2Dproj( Texture2D tex, SamplerState samp, float4 texCoords ) { return tex.Sample( samp, texCoords.xy / texCoords.w ); }
static float4 swizzleColor( float4 c ) { return c; }
static float2 vposToScreenPosTexCoord( float2 vpos ) { return vpos.xy * rpWindowCoord.xy; }


#define _dot3( a, b )  dot(a, b).xxx


#define BRANCH
#define IFANY


struct drawVert_t {
	float4 position : POSITION;
	float2 texcoord : TEXCOORD0;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 color : COLOR0;
	float4 color2 : COLOR1;
};
