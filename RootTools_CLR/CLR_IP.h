#pragma once

#include "windows.h"
#include "memoryapi.h"

#include "windows.h"
#include "memoryapi.h"
#include <stdio.h>
#include <list>
#include "CLR_IP_ParamStruct.h"
#include "..\RootTools_Cpp\\Loop_Processor.h"
#include "..\RootTools_Cpp\\IPP_Processor.h"
#include "..\RootTools_Cpp\\SIMD_Processor.h"
#include "..\RootTools_Cpp\\OpenCV_Processor.h"
#include <iostream>

using namespace System::Collections::Generic; // List

namespace RootTools_CLR
{
	public ref class CLR_IP
	{
	protected:
	public:
		// Vision
		static void Cpp_Threshold(Cpp_ThresholdParam^ param);
		static float Cpp_TemplateMatching(Cpp_TemplateParam^ param);
		static array<Cpp_LabelParam^>^ Cpp_Labeling(Cpp_LabelingParam^ param);

		// Filtering
		static void Cpp_GaussianBlur(Cpp_GaussianParam^ param);
		static void Cpp_AverageBlur(Cpp_ImageProcessParam^ param);
		static void Cpp_MedianBlur(Cpp_MedianParam^ param);
		static void Cpp_Morphology(Cpp_MorphologyParam^ param);

	};
}