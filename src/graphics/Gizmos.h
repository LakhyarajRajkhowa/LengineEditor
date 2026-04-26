#pragma once

#include "assets/MeshRegistry.h"
#include "resources/AssetManager.h"



namespace Lengine {

	struct GizmoArrows {
		Mesh* gizmoArrow = nullptr;
		GLSLProgram gizmoArrowShader;
		glm::vec3 axis;
		glm::vec4 color;
	};

	class GizmoRenderer {
	public:
		GizmoRenderer(
			AssetManager& asstmgr
		) :
			assetManager(asstmgr)
		{
			
		}


		void drawGizmoGrid(
			const glm::mat4& view,
			const glm::mat4 proj,
			const glm::vec3 pos);


		void InitGizmo() {
			initGizmoGrid();

		}

	private:
		AssetManager& assetManager;

		Mesh* gizmoGrid = nullptr;
		GLSLProgram gizmoGridShader;

		
		void initGizmoGrid();
	};
}