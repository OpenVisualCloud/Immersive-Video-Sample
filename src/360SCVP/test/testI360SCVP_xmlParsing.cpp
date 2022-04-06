/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "gtest/gtest.h"
#include <string>
#include <fstream>
#include "../360SCVPAPI.h"
#include <math.h>

#include "../../utils/safe_mem.h"
#include "../../utils/tinyxml2.h"

#define CAMERANUM 12
//#define XYZ_ORDER

using namespace tinyxml2;

namespace{
class I360SCVPTest_xmlParsing : public testing::Test {
public:

    virtual void SetUp()
    {
      memset(&param, 0, sizeof(param_360SCVP));
    }
    virtual void TearDown()
    {
    }

    param_360SCVP           param;

    typedef struct {
    float K[3][3]; // 3x3 intrinsic matrix
    float R[3][3]; // rotation matrix
    float T[3]; // translation vector
    } CamStruct;

};

TEST_F(I360SCVPTest_xmlParsing, parsing)
  {
    EulerAngle* angle;
    angle = (EulerAngle*)malloc(sizeof(EulerAngle));
    EXPECT_TRUE( angle != NULL);
    if (!(angle))
    {
        return;
    }
    memset(angle, 0, sizeof(EulerAngle));

    CamStruct Params[CAMERANUM];
	  int i, j, k;
	  FILE* txtIn;
	  char str[20];
	  double temp;
	  int camId;

    txtIn = fopen("paras.txt", "r");
	  EXPECT_TRUE( txtIn != NULL);
    if (!(txtIn))
    {
        free (angle);
        angle = NULL;
        return;
    }
    int ret = 0;

    param.usedType = E_PARSER_ONENAL;

    void* pI360SCVP = I360SCVP_Init(&param);
    EXPECT_TRUE(pI360SCVP != NULL);
    if (!pI360SCVP)
    {
        free (angle);
        angle = NULL;
        fclose (txtIn);
        I360SCVP_unInit(pI360SCVP);
        return;
    }

    for (k = 0; k < CAMERANUM; k++)
	  {
		  fscanf(txtIn, "%s",str);
		  //camera id
		  fscanf(txtIn, "%d", &camId);
		  camId -= 1;

		  //intrinsics
		  fscanf(txtIn, "%s", str);
		  fscanf(txtIn, "%lf", &temp);
		  Params[camId].K[0][0] = temp;
		  Params[camId].K[1][1] = temp;
		  fscanf(txtIn, "%lf", &temp);
		  Params[camId].K[0][2] = temp;
		  fscanf(txtIn, "%lf", &temp);
		  Params[camId].K[1][2] = temp;
		  fscanf(txtIn, "%lf", &temp);
		  Params[camId].K[0][1] = temp;

		  Params[camId].K[1][0] = 0;
		  Params[camId].K[2][0] = 0;
		  Params[camId].K[2][1] = 0;
		  Params[camId].K[2][2] = 0;

		//extrinsics
		fscanf(txtIn, "%s", str);
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				fscanf(txtIn, "%lf", &temp);
				Params[camId].R[i][j] = temp;
			}
		}
		fscanf(txtIn, "%s", str);
		for (i = 0; i < 3; i++) {
			fscanf(txtIn, "%lf", &temp);
			Params[camId].T[i] = temp;
		}
	  }

    const char* xmlPath = "./paras.xml";

	  const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
	  XMLDocument doc;
	  doc.Parse(declaration);

	  XMLElement* root = doc.NewElement("cameraPara");
	  doc.InsertEndChild(root);

	  char array[255];
	  XMLElement* child = doc.NewElement("maxNumberHorz");
	  sprintf(array, "%d", CAMERANUM);
	  XMLText* childText = doc.NewText(array);
	  child->InsertEndChild(childText);
	  root->InsertEndChild(child);

    child = doc.NewElement("maxNumberVert");
	  sprintf(array, "%d", 1);
	  childText = doc.NewText(array);
	  child->InsertEndChild(childText);
	  root->InsertEndChild(child);

	  for (int i = 1; i <= CAMERANUM; i++) {
		  char cam[20];
		  char array[255];
		  sprintf(cam, "Camera_%d_0", i-1);
		  XMLElement* cameraID = doc.NewElement(cam);
		root->InsertEndChild(cameraID);

		XMLElement* child = doc.NewElement("cameraID_x");
		  sprintf(array, "%d", i - 1);
		XMLText* childText = doc.NewText(array);
		child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);

		child = doc.NewElement("cameraID_y");
		sprintf(array, "%d", 0);
		childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("focal_x");
		sprintf(array, "%lf", Params[i - 1].K[0][0]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("focal_y");
		sprintf(array, "%f", Params[i - 1].K[1][1]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("center_x");
		sprintf(array, "%f", Params[i - 1].K[0][2]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("center_y");
		sprintf(array, "%f", Params[i - 1].K[1][2]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

      I360SCVP_Matrix2Euler(pI360SCVP, Params[i-1].R, angle);

		child = doc.NewElement("roll");
		sprintf(array, "%f", angle->roll);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("pitch");
		sprintf(array, "%f", angle->pitch);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("yaw");
		sprintf(array, "%f", angle->yaw);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("trans_x");
		sprintf(array, "%f", Params[i - 1].T[0]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("trans_y");
		sprintf(array, "%f", Params[i - 1].T[1]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("trans_z");
		sprintf(array, "%f", Params[i - 1].T[2]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("codx");
		sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("cody");
		sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("k1");
		sprintf(array, "%f", Params[i - 1].K[0][1]);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("k2");
		sprintf(array, "%f", 0.0);
	    childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		  child = doc.NewElement("k3");
		sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("k4");
		sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		child = doc.NewElement("k5");
		sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		child->InsertEndChild(childText);
		cameraID->InsertEndChild(child);

		  child = doc.NewElement("k6");
		  sprintf(array, "%f", 0.0);
		  childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);

		  child = doc.NewElement("p1");
		  sprintf(array, "%f", 0.0);
		  childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);

		  child = doc.NewElement("p2");
		  sprintf(array, "%f", 0.0);
		childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);

		  child = doc.NewElement("k2");
		  sprintf(array, "%f", 0.0);
		  childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);

		  child = doc.NewElement("metric_radius");
		  sprintf(array, "%f", 0.0);
		  childText = doc.NewText(array);
		  child->InsertEndChild(childText);
		  cameraID->InsertEndChild(child);
	  }

	doc.SaveFile(xmlPath);

    NovelViewSEI* sei;
    sei = (NovelViewSEI*)malloc(sizeof(NovelViewSEI));
    EXPECT_TRUE( sei != NULL);
    if (!(sei))
    {
        free (angle);
        angle = NULL;
        fclose (txtIn);
        I360SCVP_unInit(pI360SCVP);
        return;
    }
    uint32_t testid = 11;
    I360SCVP_xmlParsing(pI360SCVP, xmlPath, testid, 0, sei);

    if((sei->cameraID_x-testid>1e-5)||
       (abs(sei->focal_x-Params[testid].K[0][0])>1e-5)||
       (abs(sei->focal_y-Params[testid].K[0][0])>1e-5)||
       (abs(sei->center_x-Params[testid].K[0][2])>1e-5)||
       (abs(sei->center_y-Params[testid].K[1][2])>1e-5)||

       (abs(sei->trans_x-Params[testid].T[0])>1e-5)||
       (abs(sei->trans_y-Params[testid].T[1])>1e-5)||
       (abs(sei->trans_z-Params[testid].T[2])>1e-5)||

       (abs(sei->k1-Params[testid].K[0][1])>1e-5))

       ret = 1;

    free (angle);
    angle = NULL;
    fclose (txtIn);
    free (sei);
    sei = NULL;
    I360SCVP_unInit(pI360SCVP);
    EXPECT_TRUE(ret == 0);
  }
}
