/** \file    particles_io.h
    \brief   Input/output of Nbody snapshots in various formats
    \author  EV
    \date    2010-2015

    The base class, particles::BaseIOSnapshot, is used as the common interface 
    for reading and writing Nbody snapshots to disk. 
    The snapshots are provided by particles::PointMassSet.
    Derived classes implement the data storage in various formats.
    Helper routines create an instance of the class corresponding to a given 
    format string or to the actual file format.
*/

#pragma once
#include "particles_base.h"
#include <string>

namespace particles {

/// The abstract class implementing reading and writing snapshots; 
/// derived classes take the filename as the argument of the constructor.
class BaseIOSnapshot {
public:
    virtual ~BaseIOSnapshot() {};
    /** read a snapshot from the file; 
        \param[out] points is an instance of PointMassSet class,
        its contents are replaced by the loaded data.
        \return success or failure; in the latter case points may contain garbage.
    */
    virtual void readSnapshot(PointMassSet<coord::Car>& points)=0;
    /** write a snapshot to the file; return success or failure. */
    virtual void writeSnapshot(const PointMassSet<coord::Car>& points)=0;
};

/// Text file with three coordinates, possibly three velocities and mass, space or tab-separated.
class IOSnapshotText: public BaseIOSnapshot {
public:
    IOSnapshotText(const std::string &_fileName): fileName(_fileName) {};
    virtual void readSnapshot(PointMassSet<coord::Car>& points);
    virtual void writeSnapshot(const PointMassSet<coord::Car>& points);
private:
    const std::string fileName;
};

/// NEMO snapshot format.
/// reading is supported only if compiled with UNSIO library; 
/// writing is implemented by builtin routines.
class IOSnapshotNemo: public BaseIOSnapshot {
public:
    /// create the class to read or write to the file; 
    /// if writing is intended, may provide a header string and timestamp
    /// and choose whether to append to file if it already exists
    IOSnapshotNemo(const std::string &_fileName, const std::string &_header="", double _time=0, bool _append=false) :
      fileName(_fileName), header(_header), time(_time), append(_append) {};
    virtual void readSnapshot(PointMassSet<coord::Car>& points);
    virtual void writeSnapshot(const PointMassSet<coord::Car>& points);
private:
    const std::string fileName;
    const std::string header;    ///< header string which will be written to the file
    const double time;           ///< timestamp of the snapshot to write
    const bool append;           ///< whether to append to the end of file or overwrite it
};

#ifdef HAVE_UNSIO
/// GADGET snapshot format; needs UNSIO library.
class IOSnapshotGadget: public BaseIOSnapshot {
public:
    IOSnapshotGadget(const std::string &_fileName): fileName(_fileName) {};
    virtual void readSnapshot(PointMassSet<coord::Car>& points);
    virtual void writeSnapshot(const PointMassSet<coord::Car>& points);
private:
    const std::string fileName;
};
#endif

/* ------ Correspondence between file format names and types ------- */
#if 0
/// list of all available IO snapshot formats, initialized at module start 
/// according to the file format supported at compile time
extern std::vector< std::string > formatsIOSnapshot;
#endif

/// creates an instance of appropriate snapshot reader, according to the file format 
/// determined by reading first few bytes, or throw a std::runtime_error if a file doesn't exist
BaseIOSnapshot* createIOSnapshotRead (const std::string &fileName);

/// creates an instance of snapshot writer for a given format name, 
/// or throw a std::runtime_error if the format name string is incorrect or file name is empty
BaseIOSnapshot* createIOSnapshotWrite(const std::string &fileFormat, const std::string &fileName, 
    const std::string& header="", const double time=0, const bool append=false);

/// convenience function for reading an N-body snapshot in arbitrary format
inline void readSnapshot(const std::string& fileName, PointMassSet<coord::Car>& points) {
    BaseIOSnapshot* snap = createIOSnapshotRead(fileName);
    snap->readSnapshot(points);
    delete snap;
}

};  // namespace
