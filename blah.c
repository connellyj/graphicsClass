            vec3 staticLitDir = normalize(staticPos - fragPos);\
            vec3 staticRefDir = 2.0 * dot(staticLitDir, norDir) * norDir - staticLitDir;\
            float dStatic = distance(staticPos, fragPos);\
            float aStatic = staticAtt[0] + staticAtt[1] * dStatic + staticAtt[2] * dStatic * dStatic;\
            float staticDiffInt = dot(norDir, staticLitDir) / aStatic;\
            float staticSpecInt = dot(staticRefDir, camDir);\
            if (dot(staticAim, -staticLitDir) < staticCos)\
				staticDiffInt = 0.0;\
            if (staticDiffInt <= 0.0 || staticSpecInt <= 0.0)\
                staticSpecInt = 0.0;\
            float staticSdw = textureProj(textureStaticSdw, fragSdw);\
			staticDiffInt *= staticSdw;\
			staticSpecInt *= staticSdw;\
            if (staticDiffInt <= ambInt)\
                staticDiffInt = ambInt;\
			vec3 staticDiffRefl = staticDiffInt * staticCol * diffuse;\
			vec3 staticSpecRefl = pow(staticSpecInt / aStatic, shininess) * staticCol * specular;\