
// Types
/*struct MeshSegment;
typedef std::vector<std::pair<UINT, UINT>> MeshEdges;

typedef std::vector<size_t> Segment;
typedef std::shared_ptr<MeshSegment> SegmentPtr;
typedef std::vector<SegmentPtr> VecSegPtr;

//! Return edges between vertices in TriMeshf
MeshEdges getEdges(const ml::TriMeshf& mesh);
*/

typedef std::function<bool(const MeshSegment&)> SegmentFilter;
typedef vector<pair<UINT, UINT>> MeshEdges;

class Segmentor
{
public:
    vector<MeshSegment> makeFilteredSegments(const TriMeshf& mesh) const;
    virtual vector<UINT> segment(const TriMeshf& mesh) const = 0;

protected:
    static void createSegments(const TriMeshf& mesh, const vector<UINT>& ids, UINT minPoints, const SegmentFilter& filter, vector<MeshSegment*> &acceptedSegs);
    static MeshEdges getEdges(const TriMeshf& mesh);
};

class SegmentorFelzenswalb : public Segmentor
{
public:
    SegmentorFelzenswalb(float _threshold, size_t _minSegmentVertices, float _colorWeight)
        : threshold(_threshold)
        , minSegmentVertices(_minSegmentVertices)
        , colorWeight(_colorWeight) { }

    vector<UINT> segment(const TriMeshf& mesh) const;

private:
    const float threshold;
    const size_t minSegmentVertices;
    const float colorWeight;
};
