/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#pragma once

#include <windows.h>
#include <cstdint>

using Any = uint64_t;
using Void = uint64_t;
using ScrHandle = int;

using AnimScene = int;
using Blip = int;
using Cam = int;
using Entity = ScrHandle;
using FireId = int;
using Hash = unsigned int;
using Interior = int;
using ItemSet = ScrHandle;
using Object = ScrHandle;
using Ped = ScrHandle;
using PersChar = ScrHandle;
using Pickup = int;
using Player = unsigned int;
using PopZone = int;
using Prompt = int;
using PropSet = int;
using Vehicle = ScrHandle;
using Volume = ScrHandle;

#define ALIGN8 __declspec(align(8))

struct Vector3
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;
};

static_assert(sizeof(Vector3) == 24, "");

struct FeedData
{
    alignas(8) int duration; //how long to display the feed item for (milliseconds)
    alignas(8) const char* f_1; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
    alignas(8) const char* f_2; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
    alignas(8) int f_3;
    alignas(8) int f_4;  //Seems to be a struct for the UIFEED::_0xAFF5BE9BA496CE40 native. 
    alignas(8) int f_5;
    alignas(8) const char* secondary_subtitle; //Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST. Displays another subtitle once the primary once has been displayed (from FeedInfo.subtitle).
    alignas(8) int f_7;
    alignas(8) int f_8;
    alignas(8) int f_9;
    alignas(8) int f_10;
    alignas(8) int f_11;
    alignas(8) int f_12;
};
struct FeedInfo
{
    alignas(8) int f_0;
    alignas(8) const char* title; //The title of the feed item.
    alignas(8) const char* subtitle; //the main subtitle of the feed item.
    alignas(8) const char* secondary_subtitle; //the subtitle that plays after the first subtitle.  Used with natives like UIFEED::_UI_FEED_POST_THREE_TEXT_SHARD
    alignas(8) int texture_dictionary_hash; //the texture dict hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
    alignas(8) int texture_name_hash; //the texture name hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
    alignas(8) int f_6;
    alignas(8) int f_7;
};