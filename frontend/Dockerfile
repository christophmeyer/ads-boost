FROM node:16 as react-build   

COPY ./app /app
WORKDIR /app

ENV REACT_APP_ADSBOOST_PORT ""
ENV REACT_APP_ADSBOOST_PATH "/ws"
ENV REACT_APP_TILESERVER_URL "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"

RUN yarn
RUN yarn build

FROM nginx:alpine
COPY --from=react-build /app/build /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf