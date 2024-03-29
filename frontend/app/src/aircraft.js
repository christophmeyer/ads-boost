import RotatedMarker from "./rotated_marker";
import { Popup } from "react-leaflet";
import L from "leaflet";

const heliAircraftMarker = new L.icon({
  iconUrl: "aircraft_helicopter.png",
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});
const lightAircraftMarker = new L.icon({
  iconUrl: "aircraft_light.png",
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});
const med1AircraftMarker = new L.icon({
  iconUrl: "aircraft_medium_1.png",
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});
const med2AircraftMarker = new L.icon({
  iconUrl: "aircraft_medium_2.png",
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});
const heavyAircraftMarker = new L.icon({
  iconUrl: "aircraft_heavy.png",
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});

function getAircraftMarker(aircraft_category) {
  switch (aircraft_category) {
    case "LGHT":
      return lightAircraftMarker;
    case "MED1":
      return med1AircraftMarker;
    case "MED2":
      return med2AircraftMarker;
    case "HVTX":
      return heavyAircraftMarker;
    case "HEVY":
      return heavyAircraftMarker;
    case "ROTR":
      return heliAircraftMarker;
    default:
      return med2AircraftMarker;
  }
}

function Aircraft(props) {
  return (
    <RotatedMarker
      key={props.contact.icao}
      position={[props.contact.lat, props.contact.lon]}
      icon={getAircraftMarker(props.contact.aircraft_category)}
      rotationAngle={props.contact.heading}
      rotationOrigin="center"
    >
      <Popup>
        ICAO: {props.contact.icao}
        <br />
        Callsign: {props.contact.callsign}
        <br />
        Altitude: {props.contact.altitude}
        <br />
      </Popup>
    </RotatedMarker>
  );
}

export default Aircraft;
