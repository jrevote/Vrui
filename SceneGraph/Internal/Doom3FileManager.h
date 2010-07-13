/***********************************************************************
Doom3FileManager - Class to read files from sets of pk3/pk4 files and
patch directories.
Copyright (c) 2007-2010 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_INTERNAL_DOOM3FILEMANAGER_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3FILEMANAGER_INCLUDED

#include <string>
#include <vector>
#include <SceneGraph/Internal/Doom3NameTree.h>
#include <SceneGraph/Internal/Doom3PakFile.h>

namespace SceneGraph {

class Doom3FileManager
	{
	/* Embedded classes: */
	private:
	struct PakFileHandle // Structure containing data necessary to read a file from a pak archive
		{
		/* Elements: */
		Doom3PakFile* pakFile; // Pointer to the pak archive containing the file
		Doom3PakFile::FileID fileID; // Handle to access the file inside the pak archive
		
		/* Constructors and destructors: */
		PakFileHandle(Doom3PakFile* sPakFile,const Doom3PakFile::FileID& sFileID)
			:pakFile(sPakFile),fileID(sFileID)
			{
			};
		};
	
	typedef Doom3NameTree<PakFileHandle> PakFileTree; // Structure to store all files in a pak archive
	
	class DummyNameFilter // Class for name filters that always pass
		{
		/* Methods: */
		public:
		static bool filter(const std::string& fileName)
			{
			return true;
			};
		};
	
	class ExtensionFilter // Class to filter names by extension
		{
		/* Elements: */
		private:
		std::string extension;
		
		/* Constructors and destructors: */
		public:
		ExtensionFilter(const char* sExtension)
			:extension(sExtension)
			{
			};
		
		/* Methods: */
		bool filter(const std::string& fileName) const
			{
			/* Check the file extension against the filter: */
			const char* extPtr=0;
			for(const char* fnPtr=fileName.c_str();*fnPtr!='\0';++fnPtr)
				if(*fnPtr=='.')
					extPtr=fnPtr;
			return extPtr!=0&&extension==extPtr+1;
			};
		};
	
	template <class ClientFunctorParam,class NameFilterParam>
	class DirectorySearcher // Helper class to allow clients to search the directory tree
		{
		/* Elements: */
		private:
		char pathName[2048]; // Full pathname of the current file
		char* pathEndPtr; // Pointer after the end of the current directory prefix
		ClientFunctorParam& clientFunctor;
		const NameFilterParam& nameFilter;
		
		/* Constructors and destructors: */
		public:
		DirectorySearcher(ClientFunctorParam& sClientFunctor,const NameFilterParam& sNameFilter)
			:pathEndPtr(pathName),clientFunctor(sClientFunctor),nameFilter(sNameFilter)
			{
			};
		
		/* Methods: */
		void enterInteriorNode(std::string name)
			{
			/* Add the directory name to the end of the current path: */
			memcpy(pathEndPtr,name.c_str(),name.length());
			pathEndPtr+=name.length();
			*pathEndPtr='/';
			++pathEndPtr;
			};
		void leaveInteriorNode(std::string name)
			{
			/* Remove the directory name from the end of the current path: */
			pathEndPtr-=name.length()+1;
			};
		void operator()(std::string name,const PakFileHandle& pfh)
			{
			if(nameFilter.filter(name))
				{
				/* Append the file name to the end of the current path: */
				memcpy(pathEndPtr,name.c_str(),name.length()+1); // Includes the NUL terminator!
				
				/* Call the client functor: */
				clientFunctor(pathName);
				}
			};
		};
	
	/* Elements: */
	std::vector<Doom3PakFile*> pakFiles; // The list of pk3/pk4 files
	PakFileTree pakFileTree; // The tree containing the pak archive's files
	
	/* Constructors and destructors: */
	public:
	Doom3FileManager(void); // Creates an empty file manager
	Doom3FileManager(const char* baseDirectory,const char* pakFilePrefix); // Creates a file manager by loading all pk3/pk4 files that match the given prefix in the given directory
	~Doom3FileManager(void); // Destroys the file manager
	
	/* Methods: */
	void addPakFile(const char* pakFileName); // Adds a pk3/pk4 file to the file manager
	void addPakFiles(const char* baseDirectory,const char* pakFilePrefix); // Adds all pk3/pk4 files that match the given prefix from the given base directory
	template <class ClientFunctorParam>
	void searchFileTree(ClientFunctorParam& cf) const // Searches the entire file tree and calls the client functor for each file
		{
		DummyNameFilter dnf;
		DirectorySearcher<ClientFunctorParam,DummyNameFilter> ds(cf,dnf);
		pakFileTree.traverseTree(ds);
		}
	template <class ClientFunctorParam>
	void searchFileTree(ClientFunctorParam& cf,const char* extension) const // Searches the entire file tree and calls the client functor for each file that matches the given extension (case-sensitive)
		{
		ExtensionFilter ef(extension);
		DirectorySearcher<ClientFunctorParam,ExtensionFilter> ds(cf,ef);
		pakFileTree.traverseTree(ds);
		}
	template <class ClientFunctorParam,class NameFilterParam>
	void searchFileTree(ClientFunctorParam& cf,const NameFilterParam& nf) const // Searches the entire file tree and calls the client functor for each file that matches the name filter
		{
		DirectorySearcher<ClientFunctorParam,NameFilterParam> ds(cf,nf);
		pakFileTree.traverseTree(ds);
		}
	unsigned char* readFile(const char* fileName,size_t& fileSize); // Reads a file
	};

}

#endif