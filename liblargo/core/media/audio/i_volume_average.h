#ifndef I_VOLUME_AVERAGE_H
#define I_VOLUME_AVERAGE_H

namespace core
{

namespace media
{

namespace audio
{

class IAverageVolume
{
public:
    virtual ~IAverageVolume() { }

    //IAverageVolume
    virtual double GetAverageVolume() const = 0;
};

} // audio

} // media

} // core


#endif // I_VOLUME_AVERAGE_H
