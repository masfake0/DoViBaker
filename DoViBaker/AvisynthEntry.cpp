#include <windows.h>

#pragma warning(push)
#pragma warning(disable: 4512 4244 4100 693)
#include "avisynth.h"
#pragma warning(pop)

#include "DoViBaker.h"
#include <vector>
#include <string>
#include <sstream>

AVSValue __cdecl Create_RealDoViBaker(PClip blclip, PClip elclip, const char* rpuPath, bool qnd, std::string cubeFiles, std::string nits, std::string cubesBasePath, const AVSValue* args, IScriptEnvironment* env)
{
  if (!blclip->GetVideoInfo().HasVideo() || (elclip && !elclip->GetVideoInfo().HasVideo())) {
    env->ThrowError("DoViBaker: Clip not available");
  }

  if (!blclip->GetVideoInfo().IsYUV() || (elclip && !elclip->GetVideoInfo().IsYUV())) {
    env->ThrowError("DoViBaker: Clip must be in YUV format");
  }

  int chromaSubSampling = -1;
  if (blclip->GetVideoInfo().Is420() && (!elclip || elclip->GetVideoInfo().Is420())) {
    chromaSubSampling = 1;
  }
  if (blclip->GetVideoInfo().Is444() && (!elclip || elclip->GetVideoInfo().Is444())) {
    chromaSubSampling = 0;
  }
  if (chromaSubSampling<0) {
    env->ThrowError("DoViBaker: Base Layer and Enhancement Layer clips must have both the same chroma subsampling, either 444 or 420");
  }

  int quarterResolutionEl = 0;
  if (elclip) {
    quarterResolutionEl = -1;
    if ((blclip->GetVideoInfo().width == elclip->GetVideoInfo().width) && (blclip->GetVideoInfo().height == elclip->GetVideoInfo().height)) {
      quarterResolutionEl = 0;
    }
    if ((blclip->GetVideoInfo().width == 2 * elclip->GetVideoInfo().width) && (blclip->GetVideoInfo().height == 2 * elclip->GetVideoInfo().height)) {
      quarterResolutionEl = 1;
    }
    if (quarterResolutionEl < 0) {
      env->ThrowError("DoViBaker: Enhancement Layer must either be same size or quarter size as Base Layer");
    }
  }
  std::stringstream ssCubeFiles(cubeFiles);
  std::vector<std::string> cubesList;
  std::string segment;
  while (std::getline(ssCubeFiles, segment, ';'))
  {
    segment.insert(0, cubesBasePath);
    cubesList.push_back(segment);
  }
  std::stringstream ssNits(nits);
  std::vector<uint16_t> nitsList;
  while (std::getline(ssNits, segment, ';'))
  {    
    nitsList.push_back(std::atoi(segment.c_str()));
  }
  std::vector<std::pair<uint16_t, std::string>> cubeNitsPairs;
  if (cubesList.size() > 0) {
    if (cubesList.size() <= nitsList.size()) {
      env->ThrowError("DoViBaker: List of LUTs must be one entry longer then the list of nits.");
    }
    cubeNitsPairs.push_back(std::pair(0, cubesList[0]));
    for (int i = 0; i < nitsList.size(); i++) {
      cubeNitsPairs.push_back(std::pair(nitsList[i], cubesList[i + 1]));
    }
  }
  
  if (chromaSubSampling == 0 && quarterResolutionEl == 0) {
    return new DoViBaker<false, false>(blclip, elclip, rpuPath, qnd, cubeNitsPairs, env);
  }
  if (chromaSubSampling == 0 && quarterResolutionEl == 1) {
    return new DoViBaker<false, true>(blclip, elclip, rpuPath, qnd, cubeNitsPairs, env);
  }
  if (chromaSubSampling == 1 && quarterResolutionEl == 0) {
    return new DoViBaker<true, false>(blclip, elclip, rpuPath, qnd, cubeNitsPairs, env);
  }
  if (chromaSubSampling == 1 && quarterResolutionEl == 1) {
    return new DoViBaker<true, true>(blclip, elclip, rpuPath, qnd, cubeNitsPairs, env);
  }
}

AVSValue __cdecl Create_DoViBaker(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  //args.ArraySize()
  return Create_RealDoViBaker(args[0].AsClip(), args[1].AsClip(), args[2].AsString(), args[3].AsBool(false), args[4].AsString(""), args[5].AsString(""), args[6].AsString(""), &args, env);
}

const AVS_Linkage *AVS_linkage = nullptr;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{
  AVS_linkage = vectors;

  env->AddFunction("DoViBaker", "c[el]c[rpu]s[qnd]b[cubes]s[mclls]s[cubes_basepath]s", Create_DoViBaker, 0);

  return "Hey it is just a spectrogram!";
}

int main()
{
	DoViProcessor dovi("Z:/rpu.bin", NULL);
  dovi.intializeFrame(1, NULL);

}
