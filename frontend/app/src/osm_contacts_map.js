import Aircraft from "./aircraft";
import { MapContainer, TileLayer } from "react-leaflet";

function OSMContactsMap(props) {
  const tile_server_url = process.env.REACT_APP_TILESERVER_URL;

  return (
    <div id="map">
      {props.initial_position && (
        <MapContainer
          style={{ height: "100vh" }}
          center={props.initial_position}
          zoomControl={false}
          zoom={9}
        >
          <TileLayer
            url={tile_server_url}
            attribution='&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors'
          />
          <div>
            {props.contact_data &&
              props.contact_data.contacts
                .filter((contact) => !(contact.lon === 0 && contact.lat === 0))
                .filter((contact) => contact.last_seen < props.timeout)
                .map((contact) => (
                  <Aircraft key={contact.icao} contact={contact} />
                ))}
          </div>
        </MapContainer>
      )}
    </div>
  );
}

export default OSMContactsMap;
