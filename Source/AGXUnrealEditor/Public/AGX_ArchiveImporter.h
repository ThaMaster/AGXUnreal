#pragma once

class AActor;
class FString;

namespace AGX_ArchiveImporter
{
	/**
	 * Read simulation objects from the .agx archive pointed to by 'ArchivePath'
	 * and populate Unreal Editor with corresponding AGXUnreal objects.
	 *
	 * AGXUnreal objects are created in the editor from AGX Dynamics objects found
	 * in the archive. The AGX Dynamics objects are not retained, but re-created
	 * from the AGXUnreal objects on BeginPlay. This means that any object type or
	 * property not yet supported by AGXUnreal will be lost. A new AActor is
	 * created for each RigidBody found in the archive and a
	 * AGX_RigidBodyComponent is added to that AActor. A ShapeComponents is
	 * created for each shape the body has. The geometry/shape tree is not
	 * maintained, instead a flat list of ShapeComponents is created.
	 *
	 * A root AActor representing the entire .agx archive is created and returned.
	 * AActors created for the RigidBodies are children of the root AActor.
	 *
	 * @param ArchivePath Path to the .agx archive file to read.
	 # @return An AActor that has all created AGXUnreal objects as children.
	 */
	AActor* ImportAGXArchive(const FString& ArchivePath);
};
