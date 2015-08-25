
class Distribution3D
{
public:
    virtual vec3f sample() const = 0;
    virtual void output(HTMLExporter &e, const string &name) const = 0;
    virtual const vector<vec3f>& samples() const = 0;
};

class Distribution3DMemorizer : public Distribution3D
{
public:
    Distribution3DMemorizer(const vector<vec3f> &samples, const vector<string> &sampleDescriptions)
    {
        _samples = samples;
        _sampleDescriptions = sampleDescriptions;
        std::sort(_samples.begin(), _samples.end(), [](const vec3f &a, const vec3f &b) { return a.x < b.x; });

        for (UINT dimension = 0; dimension < 3; dimension++)
        {
            vector<float> sortedSamples = ml::util::map(samples, [&](const vec3f &v) { return v.array[dimension]; } );
            std::sort(sortedSamples.begin(), sortedSamples.end());
            
            if (sortedSamples.size() <= 1)
                _maxDeviation.array[dimension] = 0.0f;
            else
            {
                vector<float> deviations(sortedSamples.size() - 1);
                for (int i = 0; i < sortedSamples.size() - 1; i++)
                    deviations[i] = sortedSamples[i + 1] - sortedSamples[i];

                std::sort(deviations.begin(), deviations.end());

                int offset = ml::math::round(deviations.size() * 0.75);
                if (offset >= deviations.size())
                    offset = 0;

                _maxDeviation.array[dimension] = std::accumulate(deviations.begin() + offset, deviations.end(), 0.0f) / (deviations.size() - offset);
            }
        }

        //
        // All objects are permitted inherent deviation
        //
        _maxDeviation.x += 0.05f;
        _maxDeviation.y += 0.05f;
        _maxDeviation.z += 0.25f;
    }

    vec3f sample() const
    {
        return ml::util::randomElement(_samples) + vec3f(_maxDeviation.x * ml::RNG::global.uniform(-1.0f, 1.0f),
                                                             _maxDeviation.y * ml::RNG::global.uniform(-1.0f, 1.0f),
                                                             _maxDeviation.z * ml::RNG::global.uniform(-1.0f, 1.0f));
    }

    void output(HTMLExporter &e, const string &name) const
    {
        e.appendHeader3(name);

        e.appendBoldText("X deviation: ", to_string(_maxDeviation.x));
        e.appendBoldText("Y deviation: ", to_string(_maxDeviation.y));
        e.appendBoldText("Z deviation: ", to_string(_maxDeviation.z));

        e.appendHeader3(name + " samples");
        for (UINT sampleIndex = 0; sampleIndex < _samples.size(); sampleIndex++)
        {
            const vec3f &s = _samples[sampleIndex];
            e.appendText(to_string(s.x) + ", " + to_string(s.y) + ", " + to_string(s.z) + " " + _sampleDescriptions[sampleIndex]);
        }

        e.appendLineBreak();
    }

    const vector<vec3f>& samples() const
    {
        return _samples;
    }

private:
    vector<vec3f> _samples;
    vector<string> _sampleDescriptions;
    vec3f _maxDeviation;
};

// paired distributions like speakers sample from the pair of offsets?


//
// Distribution1D works but is not currently needed.
//
//class Distribution1D
//{
//public:
//virtual double sample() const = 0;
//};
//
//class Distribution1DMemorizer : public Distribution1D
//{
//public:
//Distribution1DMemorizer(const vector<double> &samples)
//{
//_samples = samples;
//
//vector<double> sortedSamples = samples;
//std::sort(sortedSamples.begin(), sortedSamples.end());
//double v = 0.0;
//for (int i = 0; i < sortedSamples.size() - 1; i++)
//v += sortedSamples[i + 1] - sortedSamples[i];
//if (sortedSamples.size() <= 1)
//_maxDeviation = 0.0;
//else
//_maxDeviation = v * 0.25 / (sortedSamples.size() - 1);
//}
//
//double sample() const
//{
//return ml::util::randomElement(_samples) + _maxDeviation * ml::RNG::global.uniform(-1.0, 1.0);
//}
//
//private:
//vector<double> _samples;
//double _maxDeviation;
//};