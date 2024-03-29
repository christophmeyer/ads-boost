import React, {
  useEffect,
  useRef,
  forwardRef,
  useImperativeHandle,
} from "react";
import { Marker } from "react-leaflet";
import "leaflet-rotatedmarker";

const RotatedMarker = forwardRef((props, externalRef) => {
  const markerRef = useRef(null);

  const { rotationAngle, rotationOrigin, children, ...otherProps } = props;

  useEffect(() => {
    const marker = markerRef.current;
    if (marker) {
      marker.setRotationAngle(rotationAngle);
      marker.setRotationOrigin(rotationOrigin);
    }
  }, [rotationAngle, rotationOrigin]);

  // Expose the marker ref to parent components
  useImperativeHandle(externalRef, () => ({
    getMarker: () => markerRef.current,
  }));

  return (
    <Marker ref={markerRef} {...otherProps}>
      {children}
    </Marker>
  );
});

export default RotatedMarker;
